#!/usr/bin/env python3
"""Copy Tiberian Sun data from disc zips, ISOs, or a folder into one directory.

Patterns match install-retail.ps1. On macOS an .iso is mounted with hdiutil.
"""

import argparse
import fnmatch
import os
import plistlib
import shutil
import subprocess
import sys
import tempfile
import zipfile
from pathlib import Path

GROUPS = (
    (
        "Core",
        (
            "TIBSUN.MIX",
            "CACHE.MIX",
            "CONQUER.MIX",
            "SOUNDS.MIX",
            "LOCAL.MIX",
            "MULTI.MIX",
            "PATCH.MIX",
            "PCACHE.MIX",
            "MAPS??.MIX",
            "SIDEC??.MIX",
            "SIDENC??.MIX",
            "SIDECD??.MIX",
            "SPEECH??.MIX",
            "TEMPERAT.MIX",
            "TEM.MIX",
            "SNOW.MIX",
            "SNO.MIX",
            "ISOTEMP.MIX",
            "ISOSNOW.MIX",
            "*.MAP",
        ),
    ),
    ("Movies", ("MOVIES??.MIX", "*.VQA")),
    ("Music", ("SCORES.MIX", "SCORES??.MIX")),
    (
        "Firestorm",
        (
            "EXPAND??.MIX",
            "ECACHE??.MIX",
            "E??SC??.MIX",
            "E??SCD??.MIX",
            "E??SNC??.MIX",
            "E??VOX??.MIX",
            "SOUNDS01.MIX",
            "GMENU.MIX",
            "WDT.MIX",
            "WDTVOX.MIX",
        ),
    ),
)


def die(message):
    print(message, file=sys.stderr)
    raise SystemExit(1)


def megabytes(size):
    return f"{size / (1024 * 1024):.1f}"


def group_of(name):
    upper = name.upper()
    if upper == "GAME.MAP":
        return None
    for group, patterns in GROUPS:
        for pattern in patterns:
            if fnmatch.fnmatchcase(upper, pattern):
                return group
    return None


def inside(path, parent):
    try:
        path.resolve().relative_to(parent.resolve())
    except ValueError:
        return False
    return True


def safe_extract(archive, dest):
    root = dest.resolve()
    for info in archive.infolist():
        target = (root / info.filename).resolve()
        if target != root and root not in target.parents:
            die(f"Zip entry escapes the destination: {info.filename}")
    archive.extractall(root)


def mount_iso(path):
    if shutil.which("hdiutil") is None:
        die(f"Cannot mount {path}. On Windows use install-retail.ps1, or pass a folder.")
    try:
        output = subprocess.check_output(
            ["hdiutil", "attach", "-nobrowse", "-readonly", "-plist", str(path)],
            stderr=subprocess.PIPE,
        )
    except subprocess.CalledProcessError as error:
        detail = (error.stderr or b"").decode("utf-8", "replace").strip()
        die(f"Cannot mount {path}. {detail or error}")
    listing = plistlib.loads(output)
    mounts = []
    for entity in listing.get("system-entities", []):
        mount = entity.get("mount-point")
        if mount and mount not in mounts:
            mounts.append(mount)
    if not mounts:
        die(f"Mounted {path} but found no volume.")
    return mounts


def find_isos(root):
    found = []
    for dirpath, _dirnames, filenames in os.walk(root):
        for name in filenames:
            if name.lower().endswith(".iso"):
                found.append(Path(dirpath) / name)
    return sorted(found)


def open_source(source, mounted, temps):
    path = Path(source).expanduser()
    if not path.exists():
        die(f"Source does not exist: {source}")
    suffix = path.suffix.lower()
    if suffix == ".zip":
        temp = Path(tempfile.mkdtemp(prefix="opents-retail-"))
        temps.append(temp)
        with zipfile.ZipFile(path) as archive:
            safe_extract(archive, temp)
        isos = find_isos(temp)
        if not isos:
            return [temp]
        roots = []
        for iso in isos:
            mounts = mount_iso(iso)
            mounted.extend(mounts)
            roots.extend(mounts)
        return roots
    if suffix == ".iso":
        mounts = mount_iso(path.resolve())
        mounted.extend(mounts)
        return mounts
    if path.is_dir():
        return [path]
    die(f"Source is not a folder, .zip, or .iso: {source}")


def collect(roots, target, skipped):
    selected = {}
    replaced = []
    for root in roots:
        root_path = Path(root).resolve()
        for dirpath, _dirnames, filenames in os.walk(root_path):
            for name in filenames:
                path = Path(dirpath) / name
                if target.exists() and inside(path, target):
                    continue
                group = group_of(name)
                if group is None or group in skipped:
                    continue
                key = name.upper()
                size = path.stat().st_size
                previous = selected.get(key)
                if previous is not None and previous["size"] != size:
                    replaced.append(f"{key} from {root_path}")
                selected[key] = {
                    "name": key,
                    "group": group,
                    "size": size,
                    "path": path,
                }
    return selected, replaced


def validate(selected, roots):
    if not selected:
        die(
            "No game data found in "
            + ", ".join(str(root) for root in roots)
            + ". Pass a disc zip, an ISO, or a folder that holds INSTALL/TIBSUN.MIX."
        )
    if "TIBSUN.MIX" not in selected:
        missing = [
            name
            for name in ("CACHE.MIX", "CONQUER.MIX", "SOUNDS.MIX")
            if name not in selected
        ]
        if missing:
            die(
                "The source holds neither TIBSUN.MIX nor all of CACHE.MIX, "
                "CONQUER.MIX, and SOUNDS.MIX; it is missing "
                + ", ".join(missing)
                + ". Pass the installed game folder, or the disc that holds INSTALL/TIBSUN.MIX."
            )
    elif selected["TIBSUN.MIX"]["size"] < 10 * 1024 * 1024:
        print(
            "warning: TIBSUN.MIX is "
            f"{megabytes(selected['TIBSUN.MIX']['size'])} MB. "
            "The retail and Steam copies are about 75 MB.",
            file=sys.stderr,
        )
    loose_firestorm = "TIBSUN.MIX" not in selected
    if (
        loose_firestorm
        and "EXPAND01.MIX" in selected
        and "SOUNDS01.MIX" not in selected
    ):
        die(
            "EXPAND01.MIX is present but SOUNDS01.MIX is not. "
            "The game stops at startup. Pass the folder that holds SOUNDS01.MIX, "
            "or use --no-firestorm."
        )
    firestorm = [item for item in selected.values() if item["group"] == "Firestorm"]
    if firestorm and "EXPAND01.MIX" not in selected:
        print(
            "warning: Firestorm files are present without EXPAND01.MIX. "
            "The campaign data is probably still inside the disc installer. "
            "Pass the folder that setup installs, or use --no-firestorm.",
            file=sys.stderr,
        )


def copy_files(selected, target, dry_run):
    counts = {group: 0 for group, _patterns in GROUPS}
    sizes = {group: 0 for group, _patterns in GROUPS}
    kept = 0
    if not dry_run:
        target.mkdir(parents=True, exist_ok=True)
    for item in sorted(selected.values(), key=lambda item: (item["group"], item["name"])):
        counts[item["group"]] += 1
        sizes[item["group"]] += item["size"]
        destination = target / item["name"]
        if destination.is_file() and destination.stat().st_size == item["size"]:
            kept += 1
            continue
        if dry_run:
            continue
        shutil.copyfile(item["path"], destination)
        copied = destination.stat().st_size
        if copied != item["size"]:
            die(
                f"{item['name']} holds {copied} bytes after copying, "
                f"expected {item['size']}."
            )
    return counts, sizes, kept


def report(target, counts, sizes, kept, replaced, skipped, repo):
    print(f"target        : {target}")
    for group, _patterns in GROUPS:
        if counts[group] or sizes[group]:
            print(
                f"{group:<13} : {counts[group]} file(s), {megabytes(sizes[group])} MB"
            )
    for group in ("Movies", "Music", "Firestorm"):
        if group in skipped:
            print(f"{group:<13} : skipped")
        elif counts[group] == 0:
            print(f"{group:<13} : not found in the source")
    if kept:
        print(f"kept existing : {kept} file(s) already matched")
    if replaced:
        print("replaced      : " + "; ".join(replaced))
    game = repo / "build" / "macos" / "bin" / "GameD"
    print(f'run           : {game} -DATADIR="{target}"')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("sources", nargs="+", help="A .zip, .iso, or folder. Pass Firestorm last.")
    parser.add_argument("--target", default="~/Games/OpenTS")
    parser.add_argument("--no-movies", action="store_true")
    parser.add_argument("--no-music", action="store_true")
    parser.add_argument("--no-firestorm", action="store_true")
    parser.add_argument("--dry-run", action="store_true")
    args = parser.parse_args()

    target = Path(args.target).expanduser()
    skipped = set()
    if args.no_movies:
        skipped.add("Movies")
    if args.no_music:
        skipped.add("Music")
    if args.no_firestorm:
        skipped.add("Firestorm")

    repo = Path(__file__).resolve().parents[4]
    mounted = []
    temps = []
    try:
        roots = []
        for source in args.sources:
            roots.extend(open_source(source, mounted, temps))
        selected, replaced = collect(roots, target, skipped)
        validate(selected, roots)
        counts, sizes, kept = copy_files(selected, target, args.dry_run)
        report(target, counts, sizes, kept, replaced, skipped, repo)
    finally:
        for mount in mounted:
            subprocess.run(
                ["hdiutil", "detach", mount],
                check=False,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
        for temp in temps:
            shutil.rmtree(temp, ignore_errors=True)


if __name__ == "__main__":
    main()
