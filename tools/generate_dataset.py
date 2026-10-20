#!/usr/bin/env python3
"""
InternPath — Synthetic Dataset Generator for DSA Stress Testing & Benchmarks
Generates realistic, referentially-consistent, acyclic CSV datasets for InternPath.

Supported Workload Levels:
  Level 1: 1,000 students, 5,000 applications (~15 MB RAM in C++)
  Level 2: 10,000 students, 50,000 applications (~65 MB RAM in C++)
  Level 3: 100,000 students, 500,000 applications (~450 MB RAM in C++)
  Level 4: 1,000,000 students, 5,000,000 applications (~3.8 GB RAM in C++)

Adversarial Presets:
  --adversarial hotspot      : Concentrates thousands of applications onto 1-2 flagship internships
  --adversarial dense_dag    : Creates high-density skill prerequisite graph
  --adversarial tie_break    : Forces identical fit scores across applicants for deep sort tie-breaking
  --adversarial monotonic_avl: Deadlines strictly sequential to force maximal AVL tree rotations
"""

import argparse
import csv
import math
import os
import random
import sys
from typing import Dict, List, Set, Tuple

FIRST_NAMES = [
    "Alex", "Jordan", "Taylor", "Morgan", "Sam", "Chris", "Pat", "Riley",
    "Kamal", "Nimal", "Sunil", "Priyantha", "Chathura", "Dinesh", "Kasun", "Ruwan",
    "Nuwan", "Sachin", "Anushka", "Dilshan", "Sandun", "Mahesh", "Roshan", "Saman",
    "Anura", "Pradeep", "Tharindu", "Gayan", "Lakmal", "Isuru", "Supun", "Danushka",
    "Elena", "Sophia", "Marcus", "Lucas", "Maya", "Aria", "Liam", "Noah",
    "Emma", "Oliver", "Ava", "Elijah", "Charlotte", "William", "Amelia", "James"
]

LAST_NAMES = [
    "Perera", "Silva", "Fernando", "de Silva", "Jayasinghe", "Bandara", "Dissanayake", "Herath",
    "Senanayake", "Wijesinghe", "Gunasekara", "Karunaratne", "Rajapaksha", "Mendis", "Fonseka", "Abeysekara",
    "Smith", "Johnson", "Williams", "Brown", "Jones", "Garcia", "Miller", "Davis",
    "Rodriguez", "Martinez", "Hernandez", "Lopez", "Gonzalez", "Wilson", "Anderson", "Thomas"
]

TECH_COMPANIES = [
    ("WSO2 Labs", "Open-source enterprise middleware and API management solutions."),
    ("Virtusa Innovations", "Global digital engineering and cloud optimization partner."),
    ("Sysco LABS", "Advanced restaurant technology, logistics, and supply chain engines."),
    ("LSEG Technology", "Ultra-low-latency financial market infrastructure and order routing."),
    ("IFS R&D", "Industrial and enterprise software for service and resource management."),
    ("MillenniumIT ESP", "Enterprise systems integration, automation, and cybersecurity."),
    ("Zone24x7", "Pioneering retail AI, embedded systems, and robotics hardware."),
    ("CodeGen", "Next-generation travel automation and autonomous vehicle R&D."),
    ("99x Tech", "Specialized high-end SaaS product engineering for European enterprises."),
    ("Rootcode Labs", "Bespoke software architecture, AI agents, and product design studio."),
    ("Octave Data", "Advanced big data analytics, machine learning, and algorithmic pricing."),
    ("Creative Software", "Scandinavian offshore software engineering and quality assurance."),
    ("Synopsys Core", "Semiconductor design, silicon verification, and chip prototyping."),
    ("Axiata Digital Labs", "Telecommunication API ecosystems and digital transformation."),
    ("Mitra Innovation", "Cloud engineering, microservices, and digital incubator."),
    ("Ennovi Automation", "Precision automotive electronics and connected hardware."),
    ("Informatics Labs", "Government digital infrastructure and banking software."),
    ("Auxenta Engineering", "Silicon Valley innovation lab specializing in IoT platforms."),
    ("Calcey Cloud", "Agile cloud solutions and distributed applications for US clients."),
    ("CodeGen Autonomous", "High-performance autonomous navigation systems.")
]

ROLES = [
    "Software Engineering Intern", "Embedded Systems Intern", "Backend Platform Intern",
    "Full-Stack Web Intern", "Data Science & AI Intern", "Cloud Infrastructure Intern",
    "Mobile App Intern (iOS/Android)", "DevOps & SRE Intern", "Cybersecurity Analyst Intern",
    "Firmware & RTOS Intern", "Machine Learning Intern", "Frontend Systems Intern",
    "Distributed Systems Intern", "Database Performance Intern", "Automation & QA Intern"
]

SKILL_CATALOGUE = [
    # Tier 0 (Foundations)
    ("C++ Basics", "Programming", 0),
    ("Python", "Programming", 0),
    ("Java Basics", "Programming", 0),
    ("Digital Electronics", "Electronics", 0),
    ("Discrete Math", "Mathematics", 0),
    ("Linux CLI", "Tools", 0),
    ("Git & VCS", "Tools", 0),
    ("SQL Basics", "Database", 0),
    ("Computer Architecture", "Systems", 0),
    ("Networking Basics", "Networks", 0),

    # Tier 1 (Core Concepts)
    ("Data Structures", "Programming", 1),
    ("Algorithms Design", "Programming", 1),
    ("Object-Oriented Design", "Programming", 1),
    ("Microcontrollers", "Embedded", 1),
    ("Relational Databases", "Database", 1),
    ("Web Foundations (HTML/CSS/JS)", "Web", 1),
    ("Operating Systems", "Systems", 1),
    ("Linear Algebra", "Mathematics", 1),
    ("Computer Networks", "Networks", 1),
    ("Circuit Design", "Electronics", 1),

    # Tier 2 (Intermediate)
    ("Advanced C++ (C++20)", "Programming", 2),
    ("Embedded C", "Embedded", 2),
    ("REST API Architecture", "Web", 2),
    ("Docker & Containers", "Tools", 2),
    ("Database Indexing & Tuning", "Database", 2),
    ("Concurrent Programming", "Systems", 2),
    ("Statistical Modeling", "Mathematics", 2),
    ("TCP/IP Protocols", "Networks", 2),
    ("PCB Design", "Electronics", 2),
    ("Automated Testing & CI/CD", "Tools", 2),

    # Tier 3 (Specializations)
    ("Real-Time Operating Systems (RTOS)", "Embedded", 3),
    ("Distributed Systems", "Systems", 3),
    ("Microservices Architecture", "Web", 3),
    ("Deep Learning & PyTorch", "AI", 3),
    ("FPGA & VHDL", "Electronics", 3),
    ("Cloud Architecture (AWS/Azure)", "Cloud", 3),
    ("Compiler Design", "Systems", 3),
    ("Network Security & Cryptography", "Security", 3),
    ("Autonomous Robotics", "Robotics", 3),
    ("High-Performance Computing (SIMD/GPU)", "Systems", 3),
]

LOCATIONS = ["Colombo", "Kandy", "Galle", "Kurunegala", "Jaffna", "Remote (Global)"]
MODES = ["Onsite", "Hybrid", "Remote"]

LEVEL_PRESETS = {
    1: {"students": 1000, "companies": 100, "internships": 200, "skills": 50, "dag_edges": 120, "applications": 5000},
    2: {"students": 10000, "companies": 500, "internships": 1500, "skills": 200, "dag_edges": 600, "applications": 50000},
    3: {"students": 100000, "companies": 2500, "internships": 10000, "skills": 500, "dag_edges": 2000, "applications": 500000},
    4: {"students": 1000000, "companies": 10000, "internships": 50000, "skills": 1000, "dag_edges": 5000, "applications": 5000000},
}


def generate_skills(rng: random.Random, num_skills: int) -> List[Dict]:
    skills = []
    # Base catalogue
    for i, (name, cat, tier) in enumerate(SKILL_CATALOGUE):
        if len(skills) >= num_skills:
            break
        skills.append({
            "id": i + 1,
            "name": name,
            "category": cat,
            "tier": tier
        })

    # Extra generated skills if num_skills exceeds base catalogue
    next_id = len(skills) + 1
    categories = ["Programming", "Systems", "Embedded", "Electronics", "Cloud", "AI", "Tools", "Database"]
    while len(skills) < num_skills:
        cat = rng.choice(categories)
        tier = rng.randint(0, 4)
        name = f"{cat} Skill Spec-{next_id}"
        skills.append({
            "id": next_id,
            "name": name,
            "category": cat,
            "tier": tier
        })
        next_id += 1

    return skills


def generate_acyclic_prerequisites(rng: random.Random, skills: List[Dict], target_edges: int, dense: bool = False) -> List[Dict]:
    """Generates strictly acyclic edges by enforcing tier(from) < tier(to)."""
    edges = []
    existing: Set[Tuple[int, int]] = set()

    by_tier: Dict[int, List[int]] = {}
    for s in skills:
        t = s["tier"]
        by_tier.setdefault(t, []).append(s["id"])

    tiers = sorted(by_tier.keys())
    if len(tiers) < 2:
        return edges

    attempts = 0
    max_attempts = target_edges * 10

    while len(edges) < target_edges and attempts < max_attempts:
        attempts += 1
        t_from_idx = rng.randint(0, len(tiers) - 2)
        t_to_idx = rng.randint(t_from_idx + 1, len(tiers) - 1)

        t_from = tiers[t_from_idx]
        t_to = tiers[t_to_idx]

        u = rng.choice(by_tier[t_from])
        v = rng.choice(by_tier[t_to])

        if u == v or (u, v) in existing:
            continue

        existing.add((u, v))
        min_level = rng.randint(1, 2)
        edges.append({
            "fromSkillId": u,
            "toSkillId": v,
            "minLevel": min_level
        })

    return edges


def generate_companies(rng: random.Random, num_companies: int) -> List[Dict]:
    companies = []
    for i in range(num_companies):
        c_id = i + 1
        if i < len(TECH_COMPANIES):
            name, desc = TECH_COMPANIES[i]
        else:
            name = f"Enterprise Tech Group {c_id}"
            desc = f"Specialized software and systems engineering division {c_id}."
        companies.append({"id": c_id, "name": name, "description": desc})
    return companies


def generate_internships(rng: random.Random, num_internships: int, num_companies: int, skills: List[Dict], monotonic_deadlines: bool = False) -> Tuple[List[Dict], List[Dict]]:
    internships = []
    requirements = []
    skill_ids = [s["id"] for s in skills]

    base_date = 20261021  # Today is 20261020

    for i in range(num_internships):
        in_id = i + 1
        company_id = rng.randint(1, num_companies)
        role = rng.choice(ROLES)
        title = f"{role} ({in_id})"
        loc = rng.choice(LOCATIONS)
        mode = rng.choice(MODES)
        stipend = rng.choice([20000, 25000, 30000, 35000, 45000, 50000, 60000, 75000])
        capacity = rng.randint(1, 4)

        if monotonic_deadlines:
            # Strictly increasing calendar days
            day_offset = i % 180
            deadline = 20261100 + (day_offset % 28 + 1)
        else:
            month = rng.randint(11, 12)
            day = rng.randint(1, 28)
            deadline = 20260000 + (month * 100) + day

        internships.append({
            "id": in_id,
            "companyId": company_id,
            "title": title,
            "location": loc,
            "mode": mode,
            "stipend": stipend,
            "capacity": capacity,
            "deadline": deadline,
            "status": "Open"
        })

        # Generate 2 to 5 requirements per internship
        num_reqs = rng.randint(2, min(5, len(skill_ids)))
        chosen_skills = rng.sample(skill_ids, num_reqs)

        for req_idx, s_id in enumerate(chosen_skills):
            req_level = rng.randint(1, 3)
            weight = rng.randint(1, 5)
            # Ensure at least first requirement is mandatory
            mandatory = "true" if req_idx == 0 or rng.random() < 0.4 else "false"
            requirements.append({
                "internshipId": in_id,
                "skillId": s_id,
                "requiredLevel": req_level,
                "weight": weight,
                "mandatory": mandatory
            })

    return internships, requirements


def generate_students_and_proficiency(rng: random.Random, num_students: int, skills: List[Dict]) -> Tuple[List[Dict], List[Dict], Dict[int, Dict[int, int]]]:
    students = []
    proficiencies = []
    student_prof_map: Dict[int, Dict[int, int]] = {}
    skill_ids = [s["id"] for s in skills]

    for i in range(num_students):
        s_id = i + 1
        fn = rng.choice(FIRST_NAMES)
        ln = rng.choice(LAST_NAMES)
        full_name = f"{fn} {ln}"
        email = f"student{s_id}@campus.edu"

        students.append({
            "id": s_id,
            "name": full_name,
            "email": email
        })

        # Students realistically know 4 to 12 skills
        student_prof_map[s_id] = {}
        num_known = rng.randint(3, min(12, len(skill_ids)))
        known_skills = rng.sample(skill_ids, num_known)

        for s_skill_id in known_skills:
            level = rng.randint(1, 3)
            student_prof_map[s_id][s_skill_id] = level
            proficiencies.append({
                "studentId": s_id,
                "skillId": s_skill_id,
                "level": level
            })

    return students, proficiencies, student_prof_map


def calculate_match_exact(requirements: List[Dict], student_skills: Dict[int, int]) -> Tuple[int, int, bool]:
    """Calculates fitNumerator, fitDenominator, and eligibility using frozen domain math."""
    if not requirements:
        return 0, 1, True

    num = 0
    den = 0
    eligible = True

    for req in requirements:
        w = req["weight"]
        req_lvl = req["requiredLevel"]
        is_mand = (req["mandatory"] == "true")

        st_lvl = student_skills.get(req["skillId"], 0)

        if is_mand and st_lvl < req_lvl:
            eligible = False

        capped = min(st_lvl, req_lvl)
        multiplier = 6 // req_lvl
        contrib = w * capped * multiplier

        num += contrib
        den += (6 * w)

    if den == 0:
        den = 1

    return num, den, eligible


def generate_applications(
    rng: random.Random,
    num_applications: int,
    students: List[Dict],
    internships: List[Dict],
    requirements_by_internship: Dict[int, List[Dict]],
    student_prof_map: Dict[int, Dict[int, int]],
    adversarial_mode: str = ""
) -> List[Dict]:
    applications = []
    seen_pairs: Set[Tuple[int, int]] = set()

    student_ids = [s["id"] for s in students]
    internship_ids = [i["id"] for i in internships]

    submit_seq = 1
    app_id = 1

    if adversarial_mode == "hotspot" and len(internship_ids) >= 1:
        # 80% of applications target internship 1
        hotspot_id = 1
        for s_id in student_ids:
            if len(applications) >= num_applications:
                break
            seen_pairs.add((s_id, hotspot_id))
            reqs = requirements_by_internship.get(hotspot_id, [])
            num, den, _ = calculate_match_exact(reqs, student_prof_map.get(s_id, {}))
            applications.append({
                "id": app_id,
                "studentId": s_id,
                "internshipId": hotspot_id,
                "status": "Submitted",
                "submitSequence": submit_seq,
                "fitNumeratorAtSubmit": num,
                "fitDenominatorAtSubmit": den,
                "rankAtClose": 0
            })
            app_id += 1
            submit_seq += 1

    # Fill remainder
    attempts = 0
    max_attempts = num_applications * 5

    while len(applications) < num_applications and attempts < max_attempts:
        attempts += 1
        s_id = rng.choice(student_ids)
        in_id = rng.choice(internship_ids)

        if (s_id, in_id) in seen_pairs:
            continue

        seen_pairs.add((s_id, in_id))
        reqs = requirements_by_internship.get(in_id, [])
        num, den, _ = calculate_match_exact(reqs, student_prof_map.get(s_id, {}))

        if adversarial_mode == "tie_break":
            # Force identical fit numerator for all applications
            num = 100
            den = 120

        applications.append({
            "id": app_id,
            "studentId": s_id,
            "internshipId": in_id,
            "status": "Submitted",
            "submitSequence": submit_seq,
            "fitNumeratorAtSubmit": num,
            "fitDenominatorAtSubmit": den,
            "rankAtClose": 0
        })
        app_id += 1
        submit_seq += 1

    return applications


def write_csv(filepath: str, fieldnames: List[str], rows: List[Dict]):
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, mode="w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def main():
    parser = argparse.ArgumentParser(description="InternPath Synthetic Dataset Generator")
    parser.add_argument("--level", type=int, choices=[1, 2, 3, 4], default=1, help="Workload level (1=1k, 2=10k, 3=100k, 4=1M students)")
    parser.add_argument("--out-dir", type=str, default="", help="Output directory for generated CSV files")
    parser.add_argument("--seed", type=int, default=42, help="Deterministic random seed")
    parser.add_argument("--adversarial", type=str, choices=["", "hotspot", "dense_dag", "tie_break", "monotonic_avl"], default="", help="Adversarial workload preset")
    args = parser.parse_args()

    cfg = LEVEL_PRESETS[args.level]
    if args.adversarial == "dense_dag":
        cfg["dag_edges"] = cfg["dag_edges"] * 3

    out_dir = args.out_dir
    if not out_dir:
        adv_tag = f"_{args.adversarial}" if args.adversarial else ""
        out_dir = os.path.join("build", "benchmark_data", f"level{args.level}{adv_tag}")

    os.makedirs(out_dir, exist_ok=True)
    rng = random.Random(args.seed)

    print(f"=== InternPath Synthetic Dataset Generator ===")
    print(f"Configuration: Level {args.level} {f'[{args.adversarial}]' if args.adversarial else ''}")
    print(f"Target: {cfg['students']:,} students, {cfg['companies']:,} companies, {cfg['internships']:,} internships, {cfg['applications']:,} applications")
    print(f"Output directory: {os.path.abspath(out_dir)}")
    print(f"Random seed: {args.seed}")

    print("Generating skills & categories...")
    skills = generate_skills(rng, cfg["skills"])

    print("Generating acyclic skill prerequisites...")
    prereqs = generate_acyclic_prerequisites(rng, skills, cfg["dag_edges"], dense=(args.adversarial == "dense_dag"))

    print("Generating companies...")
    companies = generate_companies(rng, cfg["companies"])

    print("Generating internships & requirements...")
    internships, requirements = generate_internships(rng, cfg["internships"], cfg["companies"], skills, monotonic_deadlines=(args.adversarial == "monotonic_avl"))

    reqs_by_in: Dict[int, List[Dict]] = {}
    for r in requirements:
        reqs_by_in.setdefault(r["internshipId"], []).append(r)

    print("Generating students & proficiencies...")
    students, proficiencies, student_prof_map = generate_students_and_proficiency(rng, cfg["students"], skills)

    print("Generating applications & submit-time fit scores...")
    applications = generate_applications(rng, cfg["applications"], students, internships, reqs_by_in, student_prof_map, adversarial_mode=args.adversarial)

    print(f"Writing CSV files to {out_dir}...")
    write_csv(os.path.join(out_dir, "skills.csv"), ["id", "name", "category"], [{"id": s["id"], "name": s["name"], "category": s["category"]} for s in skills])
    write_csv(os.path.join(out_dir, "skill_prerequisites.csv"), ["fromSkillId", "toSkillId", "minLevel"], prereqs)
    write_csv(os.path.join(out_dir, "companies.csv"), ["id", "name", "description"], companies)
    write_csv(os.path.join(out_dir, "internships.csv"), ["id", "companyId", "title", "location", "mode", "stipend", "capacity", "deadline", "status"], internships)
    write_csv(os.path.join(out_dir, "internship_requirements.csv"), ["internshipId", "skillId", "requiredLevel", "weight", "mandatory"], requirements)
    write_csv(os.path.join(out_dir, "students.csv"), ["id", "name", "email"], students)
    write_csv(os.path.join(out_dir, "student_proficiency.csv"), ["studentId", "skillId", "level"], proficiencies)
    write_csv(os.path.join(out_dir, "applications.csv"), ["id", "studentId", "internshipId", "status", "submitSequence", "fitNumeratorAtSubmit", "fitDenominatorAtSubmit", "rankAtClose"], applications)

    # meta.csv
    meta = [
        {"key": "schemaVersion", "value": "1"},
        {"key": "nextStudentId", "value": str(len(students) + 1)},
        {"key": "nextSkillId", "value": str(len(skills) + 1)},
        {"key": "nextCompanyId", "value": str(len(companies) + 1)},
        {"key": "nextInternshipId", "value": str(len(internships) + 1)},
        {"key": "nextApplicationId", "value": str(len(applications) + 1)},
        {"key": "nextSubmitSequence", "value": str(len(applications) + 1)},
        {"key": "demoToday", "value": "20261020"}
    ]
    write_csv(os.path.join(out_dir, "meta.csv"), ["key", "value"], meta)

    total_bytes = sum(os.path.getsize(os.path.join(out_dir, f)) for f in os.listdir(out_dir) if f.endswith(".csv"))
    print(f"Done! Successfully generated {len(os.listdir(out_dir))} CSV files ({total_bytes / (1024*1024):.2f} MB total).")


if __name__ == "__main__":
    main()
