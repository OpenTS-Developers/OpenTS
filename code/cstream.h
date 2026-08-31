/*******************************************************************************
 *                                O P E N  T S
 *******************************************************************************
 * SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright 2026 OpenTS contributors
 *
 * See LICENSE.md for applicable additional terms and warranty disclaimers.
 ******************************************************************************/

#pragma once

#include "ilinkstm.h"

#include <comdef.h>
#include <unknwn.h>

class CStreamClass : public IStream, public ILinkStream
{
	public:
		virtual LONG STDMETHODCALLTYPE QueryInterface(REFIID riid, LPVOID * ppvObject) override;
		virtual ULONG STDMETHODCALLTYPE AddRef(void) override;
		virtual ULONG STDMETHODCALLTYPE Release(void) override;

		virtual HRESULT STDMETHODCALLTYPE Read(void *pv, ULONG cb, ULONG *pcbRead) override;
		virtual HRESULT STDMETHODCALLTYPE Write(const void *pv, ULONG cb, ULONG *pcbWritten) override;

		virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition) override;
		virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER libNewSize) override;
		virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream *pstm, ULARGE_INTEGER cb, ULARGE_INTEGER *pcbRead, ULARGE_INTEGER *pcbWritten) override;
		virtual HRESULT STDMETHODCALLTYPE Commit(DWORD grfCommitFlags) override;
		virtual HRESULT STDMETHODCALLTYPE Revert() override;
		virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
		virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
		virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG *pstatstg, DWORD grfStatFlag) override;
		virtual HRESULT STDMETHODCALLTYPE Clone(IStream **ppstm) override;

		virtual HRESULT STDMETHODCALLTYPE Link_Stream(IUnknown *stream) override;
		virtual HRESULT STDMETHODCALLTYPE Unlink_Stream(IUnknown **stream) override;

	public:
		CStreamClass(void);
		virtual ~CStreamClass(void);

		HRESULT Compress(void *in_buffer, ULONG length);
		HRESULT Compress(void);

		enum {
			BUFFER_SIZE = 64*1024,

			// A block that will not compress comes out bigger than it went in, up to
			// the LZO worst case, and the compressed side has to hold that.
			COMP_BUFFER_SIZE = BUFFER_SIZE + BUFFER_SIZE / 16 + 64 + 3,
		};

	private:
		/*
		 * This points to the stream the compressed data actually travels over. Nothing can be
		 * read or written until one has been linked in, and the link is broken again once the
		 * stream has been committed.
		 */
		IStreamPtr StreamPtr;

		/*
		 * This is the COM reference count for this object. The stream destroys itself once
		 * the last reference to it has been released.
		 */
		LONG RefCount;

		/*
		 * These flags record which direction the stream has been committed to. The first read
		 * or write sets one of them, and from that point on the opposite operation is
		 * refused, as are seeking and resizing.
		 */
		bool IsReading;
		bool IsWriting;

		/*
		 * This is how much of the data buffer is currently in play, expressed in bytes. While
		 * reading it counts down the part of the decompressed block not yet handed out; while
		 * writing it counts up the bytes waiting to be compressed.
		 */
		int CurOffset;

		/*
		 * This is the working buffer that holds the data in its uncompressed form, one
		 * block's worth at a time.
		 */
		void *DataBuffer;

		/*
		 * This is the working buffer that holds a block in its compressed form, on its way to
		 * or from the linked stream.
		 */
		void *StreamBuffer;

		/*
		 * This is the scratch memory the LZO compressor keeps its dictionary in. It is of no
		 * interest outside the compression call itself.
		 */
		void *LZODictionary;

		/*
		 * This is the header of the block currently being read or written. Every block on the
		 * stream is preceded by one, so the reader knows how much compressed data to pull in
		 * and how far it will expand.
		 */
		struct BlockHeader {
			/*
			 * This is the number of bytes the block occupies on the stream, compressed.
			 */
			unsigned int CompSize;

			/*
			 * This is the number of bytes the block expands to once decompressed.
			 */
			unsigned int UncompSize;
		} BlockHead;
};
