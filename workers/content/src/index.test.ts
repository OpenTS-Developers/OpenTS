import assert from "node:assert/strict";
import { test } from "node:test";
import worker from "./index.ts";

const env = {
  CONTENT_KEY: "test-key",
  INSTALL_LIMIT: { limit: async () => ({ success: true }) },
  GAME: { get: async () => null },
};

const envWith = (overrides: object) => ({ ...env, ...overrides });

test("a request without the key is refused before the path and the rate limit", async () => {
  let counted = 0;
  const counting = envWith({
    INSTALL_LIMIT: {
      limit: async () => {
        counted++;
        return { success: true };
      },
    },
  });
  const response = await worker.fetch(new Request("https://content.test/files/TIBSUN.MIX"), counting);
  assert.equal(response.status, 403);
  assert.equal(counted, 0);
});

test("a request with a wrong key is refused", async () => {
  const request = new Request("https://content.test/files/TIBSUN.MIX", {
    headers: { "x-siliconsun-key": "wrong" },
  });
  const response = await worker.fetch(request, env);
  assert.equal(response.status, 403);
});

test("a request with the key reaches the bucket", async () => {
  const request = new Request("https://content.test/files/TIBSUN.MIX", {
    headers: { "x-siliconsun-key": "test-key" },
  });
  const response = await worker.fetch(request, env);
  assert.equal(response.status, 404);
});

test("a keyed request over the rate limit waits", async () => {
  const limited = envWith({ INSTALL_LIMIT: { limit: async () => ({ success: false }) } });
  const request = new Request("https://content.test/files/TIBSUN.MIX", {
    headers: { "x-siliconsun-key": "test-key" },
  });
  const response = await worker.fetch(request, limited);
  assert.equal(response.status, 429);
});
