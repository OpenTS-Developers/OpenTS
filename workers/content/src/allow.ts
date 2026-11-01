import manifest from "../manifest.json" with { type: "json" };

const files = new Map<string, string>();
for (const file of manifest.files) {
  files.set(file.name.toUpperCase(), file.name);
}

export function resolveKey(pathname: string): string | null {
  if (pathname === "/manifest.json") {
    return "manifest.json";
  }
  if (!pathname.startsWith("/files/")) {
    return null;
  }

  let name = pathname.slice("/files/".length);
  if (name.length === 0 || name.includes("/") || name.includes("\\") || name.includes("..")) {
    return null;
  }
  try {
    name = decodeURIComponent(name);
  } catch {
    return null;
  }
  if (name.includes("/") || name.includes("\\") || name.includes("..")) {
    return null;
  }
  return files.get(name.toUpperCase()) ?? null;
}
