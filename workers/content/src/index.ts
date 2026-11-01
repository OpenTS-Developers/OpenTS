import { resolveKey } from "./allow.ts";

function responseHeaders(object: R2Object): { status: number; headers: Headers } {
  const headers = new Headers();
  object.writeHttpMetadata(headers);
  headers.set("etag", object.httpEtag);
  headers.set("accept-ranges", "bytes");
  headers.set("cache-control", "private");
  if (!headers.has("content-type")) {
    headers.set("content-type", object.key.endsWith(".json") ? "application/json" : "application/octet-stream");
  }

  const range = object.range;
  if (range && "offset" in range && range.length !== undefined) {
    const start = range.offset ?? 0;
    const length = range.length;
    headers.set("content-range", `bytes ${start}-${start + length - 1}/${object.size}`);
    headers.set("content-length", String(length));
    return { status: 206, headers };
  }
  if (range && "suffix" in range) {
    const length = range.suffix;
    const start = object.size - length;
    headers.set("content-range", `bytes ${start}-${object.size - 1}/${object.size}`);
    headers.set("content-length", String(length));
    return { status: 206, headers };
  }

  headers.set("content-length", String(object.size));
  return { status: 200, headers };
}

export default {
  async fetch(request, env): Promise<Response> {
    if (request.method !== "GET" && request.method !== "HEAD") {
      return new Response("Method not allowed", {
        status: 405,
        headers: { allow: "GET, HEAD" },
      });
    }

    const key = request.headers.get("x-siliconsun-key");
    if (key !== env.CONTENT_KEY) {
      return new Response("Forbidden", { status: 403 });
    }

    const name = resolveKey(new URL(request.url).pathname);
    if (name === null) {
      return new Response("Not found", { status: 404 });
    }

    const ip = request.headers.get("CF-Connecting-IP") ?? "unknown";
    const limited = await env.INSTALL_LIMIT.limit({ key: ip });
    if (!limited.success) {
      return new Response("Too many requests", { status: 429 });
    }

    console.log(JSON.stringify({
      ua: request.headers.get("user-agent"),
      method: request.method,
      key: name,
    }));

    const object = await env.GAME.get(name, { range: request.headers });
    if (object === null) {
      return new Response("Not found", { status: 404 });
    }

    const { status, headers } = responseHeaders(object);
    if (request.method === "HEAD") {
      return new Response(null, { status, headers });
    }
    return new Response(object.body, { status, headers });
  },
} satisfies ExportedHandler<Env>;
