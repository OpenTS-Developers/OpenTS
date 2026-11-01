# Game data worker

`workers/content` is the only reader of the private R2 bucket. `GET /files/<name>` streams one freeware file and honors a `Range` header, so a download can resume. `GET /manifest.json` returns the bucket object `manifest.json`.

Every request must send an `X-Siliconsun-Key` header matching the `CONTENT_KEY` secret, or the worker answers 403 before it resolves the path or counts the rate limit. The key is the value `Content_Download_Key()` returns in `code/contentcatalog.cpp`. That value ships in the client source, so it is not a real secret: it turns away browsers and copied URLs, but anyone who reads the source can still download.

`bucket_name` in `wrangler.jsonc` is the existing bucket. The names the worker will serve are the `files` list in `manifest.json` in this directory. A request for any other name is refused before the rate limit is counted.

Each address is limited to 30 requests in 60 seconds at one Cloudflare location; a full install needs 22. That limit is not a daily cap, and a burst can pass it. The worker logs the client `User-Agent` and does not check it.

From this directory:

```
npx wrangler secret put CONTENT_KEY
npx wrangler deploy
```

Without the secret the worker answers 403 to every request. For local development, put `CONTENT_KEY` in a `.dev.vars` file here; it is git-ignored. Set `Content_Download_Root()` in `code/contentcatalog.cpp` to the origin that deploy prints. The client calls `/files/<name>` on that origin.
