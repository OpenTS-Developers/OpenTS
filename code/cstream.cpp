/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#include "always.h"

#include "cstream.h"

#include "lzo.h"

extern ULONG COMRefCount;

/// <summary>
/// Creates a compressing stream object.
/// This routine prepares the working buffers that the LZO codec needs. The object starts
/// out with no storage stream of its own to compress through.
/// </summary>
/// <remarks>Call Link_Stream to attach a storage stream before reading or writing.</remarks>
CStreamClass::CStreamClass(void) :
	StreamPtr(NULL),
	RefCount(0),
	IsReading(false),
	IsWriting(false),
	CurOffset(0),
	DataBuffer(new unsigned char[BUFFER_SIZE]),
	StreamBuffer(new unsigned char[COMP_BUFFER_SIZE]),
	// The dictionary holds pointer-wide entries, so its byte size follows the target.
	LZODictionary(new unsigned char[LZO1X_MEM_COMPRESS])
{
	BlockHead.CompSize = BUFFER_SIZE - 1;
}


/// <summary>
/// Destroys the compressing stream object.
/// Any storage stream still attached is unlinked first, so that whatever is left in the
/// work buffer is compressed out rather than lost.
/// </summary>
CStreamClass::~CStreamClass(void)
{
	IUnknown **unk = NULL;
	if (StreamPtr) {
		Unlink_Stream(unk);
	}

	delete [] LZODictionary;
	LZODictionary = NULL;
	delete [] DataBuffer;
	DataBuffer = NULL;
	delete [] StreamBuffer;
	StreamBuffer = NULL;
}


/// <summary>
/// Takes out a reference on this stream object.
/// </summary>
/// <returns>Returns with the new reference count.</returns>
ULONG CStreamClass::AddRef(void)
{
	COMRefCount++;
	return(InterlockedIncrement(&RefCount));
}


/// <summary>
/// Releases a reference to this stream object.
/// This routine will destroy the object once the last outstanding reference has been
/// given up.
/// </summary>
/// <returns>Returns with the number of references that remain.</returns>
ULONG CStreamClass::Release(void)
{
	COMRefCount--;
	ULONG i = InterlockedDecrement(&RefCount);

	if (i == 0) {
		delete this;
	}

	return(i);
}


/// <summary>
/// Fetches an alternate interface to this stream object.
/// The IUnknown, IStream and ILinkStream interfaces are the ones supported. A successful
/// query takes out a reference on this object for the caller.
/// </summary>
/// <param name="riid">The identifier of the interface being asked for.</param>
/// <param name="ppvObject">Pointer to the interface pointer to fill in.</param>
/// <returns>Returns with S_OK, or E_NOINTERFACE if the interface is not supported.</returns>
LONG CStreamClass::QueryInterface(REFIID riid, LPVOID * ppvObject)
{
	if (ppvObject == NULL) {
		return(E_POINTER);
	}

	*ppvObject = NULL;
	if (riid == IID_IUnknown) {
		*ppvObject = this;
	}
	if (riid == IID_IStream) {
		*ppvObject = (IStream *)this;
	}
	if (riid == IID_ILinkStream) {
		*ppvObject = (ILinkStream *)this;
	}
	if (*ppvObject == NULL) {
		return(E_NOINTERFACE);
	}
	//reinterpret_cast<IUnknown *>(*ppvObject)->AddRef();
	this->AddRef();
	return(S_OK);
}


/// <summary>
/// Attaches the storage stream this object compresses through.
/// Use this routine to bind the compressor to the real stream that the compressed blocks
/// will be written to or read back from. Only one stream may be attached at a time.
/// </summary>
/// <param name="stream">Pointer to the object to fetch the storage stream from.</param>
/// <returns>Returns with S_OK, or E_FAIL if a stream is already attached.</returns>
HRESULT CStreamClass::Link_Stream(IUnknown *stream)
{
	if (stream == NULL) {
		return(E_POINTER);
	}

	if (StreamPtr != NULL) {
		return(E_FAIL);
	}

	HRESULT hr = stream->QueryInterface(__uuidof(IStream), (void **)&stream);
	if (FAILED(hr)) {
		/// &StreamPtr;
		StreamPtr.Attach(NULL, false);
	} else {
		StreamPtr.Attach((IStream *)stream, false);
	}

	if (FAILED(hr) && (hr != E_NOINTERFACE)) {
		_com_issue_error(hr);
	}

	return(S_OK);
}


/// <summary>
/// Detaches the underlying storage stream from this object.
/// Anything still sitting in the work buffer is compressed out first and the storage
/// stream is committed, so the caller gets back a complete stream.
/// </summary>
/// <param name="stream">Pointer to fill in with the released stream, or NULL if the caller
/// does not want it.</param>
/// <returns>Returns with S_OK, or E_FAIL if no stream was attached. A commit failure is
/// returned as it stands.</returns>
HRESULT CStreamClass::Unlink_Stream(IUnknown **stream)
{
	Compress();

	if (StreamPtr == NULL) {
		return(E_FAIL);
	}

	if (stream != NULL) {
		StreamPtr->AddRef();
		*stream = StreamPtr;
	}

	HRESULT hr = StreamPtr->Commit(0);
	if (SUCCEEDED(hr)) {
		StreamPtr.Release();
	} else {
		return(hr);
	}

	return(S_OK);
}


/// <summary>
/// Reads and decompresses data from the underlying stream.
/// This routine pulls whole compressed blocks out of the storage stream and doles out
/// pieces of the decompressed result until the caller's request has been satisfied.
/// </summary>
/// <param name="pv">Pointer to the buffer to fill with the data read.</param>
/// <param name="cb">The number of bytes to read.</param>
/// <param name="pcbRead">Pointer to fill in with the number of bytes read, or NULL if the
/// count is not wanted.</param>
/// <returns>Returns with S_OK, or an error code if the data could not be read.</returns>
/// <remarks>A stream that is being written cannot also be read.</remarks>
HRESULT CStreamClass::Read(void *pv, ULONG cb, ULONG *pcbRead)
{
	int read_size;
	int left;

	read_size = cb;
	left = cb;

	if (pv == NULL) {
		return(E_POINTER);
	}

	if (read_size < 0) {
		return(E_INVALIDARG);
	}

	if (StreamPtr == NULL) {
		return(E_FAIL);
	}

	if (IsWriting) {
		return(E_FAIL);
	}

	IsReading = true;

	if (pcbRead != NULL) {
		*pcbRead = 0;
	}

	while (left > 0) {

		int offset = CurOffset;
		if (offset > 0) {
			int len = left;
			if (left >= offset) {
				len = CurOffset;
			}
			memmove(pv, (char *)DataBuffer + BlockHead.UncompSize - offset, len);
			pv = (char *)pv + len;
			left -= len;
			CurOffset -= len;
		}

		if (left == 0) {
			break;
		}

		ULONG read = 0;

		HRESULT hr = StreamPtr->Read(&BlockHead, sizeof(BlockHead), &read);
		if (FAILED(hr)) {
			return(hr);
		}

		if (read != sizeof(BlockHead)) {
			return(E_FAIL);
		}

		hr = StreamPtr->Read(StreamBuffer, BlockHead.CompSize, &read);
		if (FAILED(hr)) {
			return(hr);
		}

		unsigned int inlen = BlockHead.CompSize;
		if (read != inlen) {
			return(E_FAIL);
		}
		lzo_byte *out = (lzo_byte *)DataBuffer;
		lzo_byte *in = (lzo_byte *)StreamBuffer;
		unsigned int out_len = BUFFER_SIZE;
		lzo1x_decompress(in, inlen, out, &out_len, 0);
		CurOffset = BlockHead.UncompSize;
	}

	if (pcbRead != NULL) {
		*pcbRead = read_size;
	}

	return(S_OK);
}


/// <summary>
/// Compresses data out to the underlying stream.
/// This routine gathers the caller's data into a work buffer and hands it to the
/// compressor a block at a time, so what reaches the storage stream is a run of
/// compressed blocks rather than the raw bytes.
/// </summary>
/// <param name="pv">Pointer to the data to write.</param>
/// <param name="cb">The number of bytes to write.</param>
/// <param name="pcbWritten">Pointer to fill in with the number of bytes accepted, or NULL
/// if the count is not wanted.</param>
/// <returns>Returns with S_OK, or an error code if the data could not be written.</returns>
/// <remarks>A stream that is being read cannot also be written. The trailing partial
/// block does not reach the storage stream until the object is flushed or unlinked.</remarks>
HRESULT CStreamClass::Write(const void *pv, ULONG cb, ULONG *pcbWritten)
{
	unsigned char *ptr;
	int write_size;
	int left;
	int result;
	int temp_size;

	ptr = (unsigned char *)pv;
	write_size = cb;
	left = cb;

	if (pv == NULL) {
		return(E_POINTER);
	}

	if (write_size < 0) {
		return(E_INVALIDARG);
	}

	if (StreamPtr == NULL) {
		return(E_FAIL);
	}

	if (IsReading) {
		return(E_FAIL);
	}

	IsWriting = true;

	if (cb != 0) {
		if (pcbWritten != NULL) {
			*pcbWritten = 0;
		}

		if (CurOffset > 0) {
			if (write_size >= BUFFER_SIZE - CurOffset) {
				write_size = BUFFER_SIZE - CurOffset;
			}

			memmove((unsigned char *)DataBuffer + CurOffset, ptr, write_size);
			temp_size = write_size + CurOffset;

			ptr += write_size;
			left -= write_size;
			CurOffset = temp_size;

			if (CurOffset == BUFFER_SIZE) {
				result = Compress(DataBuffer, CurOffset);
				if (result < 0) {
					return(result);
				}
				CurOffset = 0;
			}

			write_size = cb;
		}
		while (left >= BUFFER_SIZE) {
			result = Compress(ptr, BUFFER_SIZE);
			if (result < 0) {
				return(result);
			}
			left -= BUFFER_SIZE;
			ptr += BUFFER_SIZE;
		}

		if (left > 0) {
			memmove((unsigned char *)DataBuffer, ptr, left);
			write_size = cb;
			CurOffset = left;
		}

		if (pcbWritten) {
			*pcbWritten = write_size;
		}
	}

	return(S_OK);
}


/// <summary>
/// Moves the file pointer of the underlying stream.
/// </summary>
/// <returns>Returns with S_OK, or E_FAIL if a transfer is already under way.</returns>
/// <remarks>Seeking is refused once reading or writing has begun, since the compressor
/// keeps state that a seek would invalidate.</remarks>
HRESULT CStreamClass::Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition)
{
	if (IsReading || IsWriting) {
		return(E_FAIL);
	}

	return(StreamPtr->Seek(dlibMove, dwOrigin, plibNewPosition));
}


/// <summary>
/// Sets the size of the underlying stream.
/// </summary>
/// <returns>Returns with S_OK, or E_FAIL if a transfer is already under way.</returns>
/// <remarks>Resizing is refused once reading or writing has begun.</remarks>
HRESULT CStreamClass::SetSize(ULARGE_INTEGER libNewSize)
{
	if (IsReading || IsWriting) {
		return(E_FAIL);
	}

	return(StreamPtr->SetSize(libNewSize));
}


/// <summary>
/// Copies data from this stream over to another stream.
/// The request is handed straight to the underlying stream, so it is the compressed
/// bytes that get copied rather than the data they stand for.
/// </summary>
/// <returns>Returns with the result of the underlying stream's copy request.</returns>
HRESULT CStreamClass::CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten)
{
	return(StreamPtr->CopyTo(pstm, cb, pcbRead, pcbWritten));
}


/// <summary>
/// Commits any pending changes to the underlying stream.
/// </summary>
/// <returns>Returns with the result of the underlying stream's commit request.</returns>
HRESULT CStreamClass::Commit(DWORD grfCommitFlags)
{
	return(StreamPtr->Commit(grfCommitFlags));
}

/// <summary>
/// Discards any uncommitted changes to the stream.
/// </summary>
/// <returns>Returns with the result of the underlying stream's revert request.</returns>
HRESULT CStreamClass::Revert(void)
{
	return(StreamPtr->Revert());
}


/// <summary>
/// Locks a byte range of the underlying stream.
/// </summary>
/// <returns>Returns with the result of the underlying stream's lock request.</returns>
HRESULT CStreamClass::LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return(StreamPtr->LockRegion(libOffset, cb, dwLockType));
}


/// <summary>
/// Releases a lock on a byte range of the underlying stream.
/// </summary>
/// <returns>Returns with the result of the underlying stream's unlock request.</returns>
HRESULT CStreamClass::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return(StreamPtr->UnlockRegion(libOffset, cb, dwLockType));
}


/// <summary>
/// Fetches the statistics of the underlying stream.
/// </summary>
/// <returns>Returns with the result of the underlying stream's stat request.</returns>
HRESULT CStreamClass::Stat(STATSTG *pstatstg, DWORD grfStatFlag)
{
	return(StreamPtr->Stat(pstatstg, grfStatFlag));
}


/// <summary>
/// Creates a second stream object over the same storage.
/// The clone is made by the underlying stream, so it is a plain stream rather than a
/// compressing one.
/// </summary>
/// <returns>Returns with the result of the underlying stream's clone request.</returns>
HRESULT CStreamClass::Clone(IStream **ppstm)
{
	return(StreamPtr->Clone(ppstm));
}


/// <summary>
/// Compresses a buffer out as a single stream block.
/// This is the low level routine that runs the buffer through the LZO compressor and
/// writes the block header and the compressed bytes to the underlying stream.
/// </summary>
/// <param name="in_buffer">Pointer to the data to compress.</param>
/// <param name="length">The number of bytes to compress.</param>
/// <returns>Returns with S_OK, or an error code if the block could not be written.</returns>
HRESULT CStreamClass::Compress(void *in_buffer, ULONG length)
{
	HRESULT hr;
	unsigned int out_len = length;
	lzo1x_1_compress((lzo_byte *)in_buffer, length, (lzo_byte *)StreamBuffer, &out_len, (lzo_byte *)LZODictionary);
	// The final block of a stream is usually partial, and the reader takes this
	// header as the number of bytes the block expands to.
	BlockHead.UncompSize = length;
	length = 0;
	BlockHead.CompSize = out_len;

	hr = StreamPtr->Write(&BlockHead, sizeof(BlockHead), &length);

	if (SUCCEEDED(hr)) {
		if (length != sizeof(BlockHead)) {
			return(E_FAIL);
		}

		hr = StreamPtr->Write(StreamBuffer, out_len, &length);
		if (SUCCEEDED(hr)) {
			hr = length != out_len ? (unsigned int)E_FAIL : 0;
		}
	}

	return(hr);
}


/// <summary>
/// Flushes any buffered data out as a compressed block.
/// Use this routine to make sure the tail end of a write actually reaches the stream.
/// It does nothing if there is nothing left over to flush.
/// </summary>
/// <returns>Returns with S_OK, or an error code if the block could not be written.</returns>
HRESULT CStreamClass::Compress(void)
{
	if (IsWriting && CurOffset > 0) {
		if (StreamPtr == NULL) {
			return(E_FAIL);
		}
		if (CurOffset > 0) {
			return(Compress(DataBuffer, CurOffset));
		}
	}
	return(S_OK);
}
