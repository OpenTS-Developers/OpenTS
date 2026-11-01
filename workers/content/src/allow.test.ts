import assert from "node:assert/strict";
import { test } from "node:test";
import manifest from "../manifest.json" with { type: "json" };
import { resolveKey } from "./allow.ts";

test("every manifest file is served under its own name", () => {
  assert.equal(manifest.files.length, 22);
  for (const file of manifest.files) {
    assert.equal(resolveKey("/files/" + file.name), file.name);
    assert.equal(resolveKey("/files/" + file.name.toLowerCase()), file.name);
  }
});

test("the manifest itself is the only other object", () => {
  assert.equal(resolveKey("/manifest.json"), "manifest.json");
  assert.equal(resolveKey("/files/../manifest.json"), null);
  assert.equal(resolveKey("/files/TIBSUN.MIX/../PATCH.MIX"), null);
  assert.equal(resolveKey("/files/%2e%2e%2fmanifest.json"), null);
  assert.equal(resolveKey("/TIBSUN.MIX"), null);
});
