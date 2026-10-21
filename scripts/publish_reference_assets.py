#!/usr/bin/env python3
"""Extract public editorials and reference programs from the 88 author packages."""

from __future__ import annotations

import argparse
import json
import shutil
import zipfile
from dataclasses import dataclass
from pathlib import Path, PurePosixPath


@dataclass(frozen=True)
class Source:
    problem_id: int
    archive: str
    prefix: str = ""


SOURCES = [
    Source(1, "coast_boundary_1e5_author_package.zip"),
    Source(2, "dual_end_sum_author_package.zip"),
    Source(3, "full_problem_package.zip"),
    Source(4, "relic_full_package_sum1e6 (1).zip"),
    Source(5, "two_way_inspection_author_package.zip"),
    Source(6, "mex_crown_setter_package.zip"),
    Source(7, "guiliu_500k_author_package.zip"),
    Source(8, "lattice_prefix_full_package.zip"),
    Source(9, "ring_transfer_setter_package.zip"),
    Source(10, "twin_reduction_2m_author_package.zip"),
    Source(11, "samelevel_transit_full.zip"),
    Source(12, "dual_order_knapsack_full.zip"),
    Source(13, "dark_queue_setter_package.zip"),
    Source(14, "tree_harbor_full_package.zip"),
    Source(15, "cloud_stairs_k14_author_package.zip"),
    Source(16, "growing_tree_dfs_uoj_author_package.zip"),
    Source(17, "echo_coloring_setter_package.zip"),
    Source(18, "lake_reachability_setter_package.zip"),
    Source(19, "xor_interval_space_full_package.zip"),
    Source(20, "uoj_3743_BJ_author_package.zip", "B/"),
    Source(21, "uoj_3743_BJ_author_package.zip", "J/"),
    Source(22, "four_problems_n1000_author.zip", "p1603/"),
    Source(23, "four_problems_n1000_author.zip", "p8956/"),
    Source(24, "four_problems_n1000_author.zip", "p5367/"),
    Source(25, "four_problems_n1000_author.zip", "p2586/"),
    Source(26, "dual_queue_author_package.zip"),
    Source(27, "shenyang_B500_G100000_complete.zip", "B/"),
    Source(28, "shenyang_B500_G100000_complete.zip", "G/"),
    Source(29, "queue_sorting_author.zip"),
    Source(30, "oracle_n100_author.zip"),
    Source(31, "ballmachine_c27_author_package.zip"),
    Source(32, "directional_shrink_author_package.zip"),
    Source(33, "uoj_complete_problem_package_coprime_product_sum.zip"),
    Source(34, "odd_unification_author_package.zip"),
    Source(35, "star_energy_setter_full.zip"),
    Source(36, "collection_author_package.zip"),
    Source(37, "tree_shadow_uoj_author_package.zip"),
    Source(38, "spectral_rebalance_author_package.zip"),
    Source(39, "ring_complement_sum_setter_package.zip"),
    Source(40, "arch_bridge_uoj_author_package.zip"),
    Source(41, "fence_beacons_author_package.zip"),
    Source(42, "xor_threshold_matching_full_package.zip"),
    Source(43, "visibility_coverage_author_package.zip"),
    Source(44, "tree_edge_permutation_full_package.zip"),
    Source(45, "traffic_hub_rebuild_author_package.zip"),
    Source(46, "twin_lane_streaks_uoj_full.zip"),
    Source(47, "common_bracket_arcs_setter_package.zip"),
    Source(48, "double_balance_complete_package.zip"),
    Source(49, "shifted_product_full_author_package.zip"),
    Source(50, "random_path_elimination_full_package.zip"),
    Source(51, "four_vertex_tree_author_package.zip"),
    Source(52, "ball-peak-author-package.zip"),
    Source(53, "dense-edge-palette-author-package.zip"),
    Source(54, "frontier-map-uoj-author-package.zip"),
    Source(55, "tree-mirror-convergence-author-package.zip"),
    Source(56, "final_defense_author_package.zip"),
    Source(57, "corner_rescue_full_package.zip"),
    Source(58, "uoj_180_stacks_author_package.zip"),
    Source(59, "double_segment_sort_author_package.zip"),
    Source(60, "unique_victory_uoj_full_package.zip"),
    Source(61, "tricolor_welcome_full_package.zip"),
    Source(62, "quota_upgrade_full_package.zip"),
    Source(63, "night_bridge_schedule_full_package.zip"),
    Source(64, "under_epilogue_full_package.zip"),
    Source(65, "starlit_string_full_package_verified.zip"),
    Source(66, "angle_beats_full_package.zip"),
    Source(67, "apple_tree_full_package.zip"),
    Source(68, "cakes_full_package.zip"),
    Source(69, "spaceship_author_package.zip"),
    Source(70, "double_sort_uoj_full_package.zip"),
    Source(71, "staple_stable_uoj_full_package.zip"),
    Source(72, "matching_uoj_full_package.zip"),
    Source(73, "two_spanning_trees_uoj_full_package.zip"),
    Source(74, "xunji_huanyuan_full_package.zip"),
    Source(75, "uoj_ordered_tree_setter_package.zip"),
    Source(76, "grid_toll_organizer_package.zip"),
    Source(77, "elevator_duty_setter_package.zip"),
    Source(78, "interval_warming_full_package.zip"),
    Source(79, "walk_variance_setter_full.zip"),
    Source(80, "after_one_round_full_package.zip"),
    Source(81, "uoj_doubles_setter_package.zip"),
    Source(82, "phase_lock_full_package.zip"),
    Source(83, "interval_echo_full_package.zip"),
    Source(84, "rotor_sync_full_package.zip"),
    Source(85, "parity_echo_setter_package.zip"),
    Source(86, "echo_trace_author_package.zip"),
    Source(87, "maze_locator_setter_package.zip"),
    Source(88, "cycle_beacons_30_setter_package.zip"),
]


def parse_args() -> argparse.Namespace:
    repository = Path(__file__).resolve().parents[1]
    workspace = repository.parent
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--search-root",
        type=Path,
        action="append",
        default=[],
        help="directory to search recursively for author-package ZIPs (repeatable)",
    )
    parser.add_argument("--catalog", type=Path, default=repository / "site/data/problems.json")
    parser.add_argument("--output", type=Path, default=repository / "site/references")
    parser.add_argument("--report", type=Path, default=repository / "site/data/reference-assets.json")
    args = parser.parse_args()
    if not args.search_root:
        args.search_root = [
            workspace / "output",
            workspace / "deliverables",
            workspace / "artifacts",
            Path.home() / "Downloads",
            Path.home() / "Documents/ChatGPT/competitive programming",
            Path.home() / "Documents/ChatGPT/frontier-cs",
        ]
    return args


def archive_index(roots: list[Path]) -> dict[str, list[Path]]:
    index: dict[str, list[Path]] = {}
    for root in roots:
        if not root.is_dir():
            continue
        for path in root.rglob("*.zip"):
            index.setdefault(path.name.casefold(), []).append(path)
    return index


def select_archive(index: dict[str, list[Path]], name: str) -> Path:
    candidates = index.get(name.casefold(), [])
    if not candidates:
        raise FileNotFoundError(f"archive not found: {name}")
    # Prefer the shortest stable source path when duplicate release copies exist.
    return sorted(candidates, key=lambda path: (len(path.parts), len(str(path)), str(path)))[0]


def within_prefix(entry: zipfile.ZipInfo, prefix: str) -> bool:
    return not entry.is_dir() and entry.filename.casefold().startswith(prefix.casefold())


def editorial_score(name: str) -> tuple[int, int, str]:
    basename = PurePosixPath(name).name.casefold()
    priority = {"solution.md": 40, "editorial.md": 35, "writeup.md": 30}.get(basename, 0)
    return (priority, -name.count("/"), name)


def program_score(name: str) -> tuple[int, int, int, str]:
    path = PurePosixPath(name)
    basename = path.name.casefold()
    priority = {
        "std.cpp": 50,
        "std17.cpp": 48,
        "std20.cpp": 46,
        "confirmed_std.cpp": 44,
        "solution.cpp": 40,
        "reference.cpp": 35,
    }.get(basename, 0)
    lowered = name.casefold()
    organizer = 20 if any(part in lowered for part in ("admin/", "author/")) else 0
    source = 10 if any(part in lowered for part in ("/src/", "/source", "/solutions/", "/programs/")) else 0
    return (priority, organizer + source, -name.count("/"), name)


def choose_entries(archive: zipfile.ZipFile, prefix: str) -> tuple[str | None, str | None]:
    entries = [entry for entry in archive.infolist() if within_prefix(entry, prefix)]
    editorials = [
        entry.filename
        for entry in entries
        if PurePosixPath(entry.filename).name.casefold()
        in {"solution.md", "editorial.md", "writeup.md"}
    ]
    programs = [
        entry.filename
        for entry in entries
        if PurePosixPath(entry.filename).name.casefold()
        in {
            "std.cpp",
            "std17.cpp",
            "std20.cpp",
            "confirmed_std.cpp",
            "solution.cpp",
            "reference.cpp",
        }
    ]
    editorial = max(editorials, key=editorial_score) if editorials else None
    program = max(programs, key=program_score) if programs else None
    return editorial, program


def extract_text(archive: zipfile.ZipFile, entry: str, destination: Path) -> None:
    data = archive.read(entry)
    text = None
    for encoding in ("utf-8-sig", "utf-8", "gb18030"):
        try:
            text = data.decode(encoding)
            break
        except UnicodeDecodeError:
            continue
    if text is None:
        raise UnicodeDecodeError("utf-8", data, 0, len(data), f"cannot decode {entry}")
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    normalized = "\n".join(line.rstrip() for line in normalized.splitlines()).rstrip() + "\n"
    destination.write_text(normalized, encoding="utf-8", newline="\n")


def main() -> None:
    args = parse_args()
    catalog = json.loads(args.catalog.read_text(encoding="utf-8"))
    titles = {int(problem["id"]): problem["title"] for problem in catalog}
    if set(titles) != {source.problem_id for source in SOURCES}:
        raise ValueError("catalog IDs and reference-source IDs do not match")

    index = archive_index(args.search_root)
    staging = args.output.with_name(f".{args.output.name}.staging")
    if staging.exists():
        shutil.rmtree(staging)
    staging.mkdir(parents=True)
    report: list[dict] = []

    try:
        for source in SOURCES:
            archive_path = select_archive(index, source.archive)
            problem_dir = staging / f"{source.problem_id:03d}"
            problem_dir.mkdir()
            with zipfile.ZipFile(archive_path) as archive:
                editorial, program = choose_entries(archive, source.prefix)
                if editorial:
                    extract_text(archive, editorial, problem_dir / "solution.md")
                if program:
                    extract_text(archive, program, problem_dir / "std.cpp")
            assets = {}
            if editorial:
                assets["solution"] = f"./references/{source.problem_id:03d}/solution.md"
            if program:
                assets["standardSolution"] = f"./references/{source.problem_id:03d}/std.cpp"
            report.append(
                {
                    "id": source.problem_id,
                    "title": titles[source.problem_id],
                    "assets": assets,
                    "sourceArchive": source.archive,
                    "sourceEntries": {"solution": editorial, "standardSolution": program},
                }
            )
        if args.output.exists():
            shutil.rmtree(args.output)
        staging.replace(args.output)
    except Exception:
        shutil.rmtree(staging, ignore_errors=True)
        raise

    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    report_by_id = {int(item["id"]): item["assets"] for item in report}
    for problem in catalog:
        problem["referenceAssets"] = report_by_id[int(problem["id"])]
    args.catalog.write_text(json.dumps(catalog, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

    solutions = sum("solution" in item["assets"] for item in report)
    programs = sum("standardSolution" in item["assets"] for item in report)
    print(f"published {solutions}/88 editorials and {programs}/88 reference programs")


if __name__ == "__main__":
    main()
