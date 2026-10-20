// InternPath — Modern Enterprise Client Application (Braintrust Inspired)

const appState = {
    activeStudentId: 1,
    activeView: 'overview',
    students: [],
    skills: [],
    companies: [],
    internships: [],
    recommendations: [],
    applications: [],
    selectedExploreId: null,
    selectedRoadmapId: null,
    modeFilter: 'ALL',
    searchQuery: '',
    demoToday: 20261020
};

// --- Initialization ---
document.addEventListener('DOMContentLoaded', async () => {
    await fetchInitialData();
});

// --- Toast Feedback ---
function showToast(message, type = 'normal') {
    const container = document.getElementById('toastContainer');
    if (!container) return;

    const toast = document.createElement('div');
    toast.className = 'toast';
    toast.innerHTML = `
        <span>${type === 'error' ? '⚠' : '✓'}</span>
        <span>${message}</span>
    `;

    container.appendChild(toast);
    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateY(8px)';
        setTimeout(() => toast.remove(), 250);
    }, 2800);
}

// --- Navigation Controller ---
function switchNav(viewName) {
    appState.activeView = viewName;

    // Update sidebar nav buttons
    document.querySelectorAll('.nav-item').forEach(btn => {
        btn.classList.toggle('active', btn.getAttribute('data-view') === viewName);
    });

    // Update panels
    document.querySelectorAll('.view-panel').forEach(panel => {
        panel.classList.toggle('active', panel.id === `view-${viewName}`);
    });

    // Render corresponding view
    if (viewName === 'overview') renderOverview();
    if (viewName === 'explore') renderExplore();
    if (viewName === 'roadmap') renderRoadmap();
    if (viewName === 'skills') renderSkills();
    if (viewName === 'applications') renderApplications();
    if (viewName === 'admin') renderAdmin();
}

// --- Data Fetching ---
async function fetchInitialData() {
    try {
        const [health, students, internships, companies, skills] = await Promise.all([
            fetch('/api/health').then(r => r.json()).catch(() => null),
            fetch('/api/students').then(r => r.json()).catch(() => []),
            fetch('/api/internships').then(r => r.json()).catch(() => []),
            fetch('/api/companies').then(r => r.json()).catch(() => []),
            fetch('/api/skills').then(r => r.json()).catch(() => ({ skills: [] }))
        ]);

        if (health && health.today) appState.demoToday = health.today;
        appState.students = Array.isArray(students) ? students : [];
        appState.internships = Array.isArray(internships) ? internships : [];
        appState.companies = Array.isArray(companies) ? companies : [];
        appState.skills = (skills && skills.skills) ? skills.skills : [];

        if (appState.students.length > 0 && !appState.students.some(s => s.id === appState.activeStudentId)) {
            appState.activeStudentId = appState.students[0].id;
        }

        if (appState.internships.length > 0) {
            appState.selectedExploreId = appState.internships[0].id;
            appState.selectedRoadmapId = appState.internships[0].id;
        }

        renderSidebarProfile();
        await refreshStudentContext();
    } catch (err) {
        console.error('Failed to load initial data:', err);
        showToast('Error connecting to backend services', 'error');
    }
}

async function refreshStudentContext() {
    if (!appState.activeStudentId) return;

    try {
        const [recs, apps, studentDetail] = await Promise.all([
            fetch(`/api/students/${appState.activeStudentId}/recommendations`).then(r => r.json()).catch(() => []),
            fetch(`/api/students/${appState.activeStudentId}/applications`).then(r => r.json()).catch(() => []),
            fetch(`/api/students/${appState.activeStudentId}`).then(r => r.json()).catch(() => null)
        ]);

        appState.recommendations = Array.isArray(recs) ? recs : [];
        appState.applications = Array.isArray(apps) ? apps : [];

        // Update local student model
        const idx = appState.students.findIndex(s => s.id === appState.activeStudentId);
        if (idx !== -1 && studentDetail) {
            appState.students[idx] = studentDetail;
        }

        renderSidebarProfile();
        switchNav(appState.activeView);
    } catch (err) {
        console.error('Error refreshing candidate context:', err);
    }
}

// --- Sidebar Profile Switcher ---
function renderSidebarProfile() {
    const student = appState.students.find(s => s.id === appState.activeStudentId);
    const avatarEl = document.getElementById('userAvatarText');
    const nameEl = document.getElementById('userDisplayName');
    const selectEl = document.getElementById('sidebarStudentSelect');

    if (student) {
        if (avatarEl) {
            const initials = student.name.split(' ').map(w => w[0]).join('').substring(0, 2).toUpperCase();
            avatarEl.textContent = initials || 'ST';
        }
        if (nameEl) nameEl.textContent = student.name;
    }

    if (selectEl) {
        selectEl.innerHTML = appState.students.map(s => `
            <option value="${s.id}" ${s.id === appState.activeStudentId ? 'selected' : ''}>
                ${s.name} (ID: ${s.id})
            </option>
        `).join('');
    }
}

async function handleStudentSwitch(studentIdStr) {
    appState.activeStudentId = parseInt(studentIdStr, 10);
    await refreshStudentContext();
    showToast('Switched candidate workspace profile');
}

// --- VIEW 1: Student Overview (Braintrust Style) ---
function renderOverview() {
    const student = appState.students.find(s => s.id === appState.activeStudentId);
    const greetingEl = document.getElementById('greetingHeading');
    if (greetingEl) {
        greetingEl.textContent = `Hi, ${student ? student.name : 'Candidate'}!`;
    }

    const grid = document.getElementById('overviewRecommendedGrid');
    if (!grid) return;

    if (appState.recommendations.length === 0) {
        grid.innerHTML = `
            <div style="grid-column: 1 / -1; padding: 3rem; text-align: center; background: white; border: 1px solid var(--border-card); border-radius: var(--radius-xl);">
                <h3 style="color: var(--text-heading); margin-bottom: 0.5rem;">No recommendations generated</h3>
                <p style="color: var(--text-muted);">Declare your engineering skills in the "My Skills" tab to unlock matches.</p>
            </div>
        `;
        return;
    }

    grid.innerHTML = appState.recommendations.map(r => {
        const hasApplied = appState.applications.some(a => a.internshipId === r.internshipId && a.status !== 'Withdrawn');
        const formattedDeadline = formatDeadline(r.deadline);

        return `
            <div class="job-card" onclick="openExploreDetail(${r.internshipId})">
                <div class="job-card-top">
                    <div class="job-brand-group">
                        <div class="company-avatar">${(r.companyName || 'C').charAt(0)}</div>
                        <div class="job-headings">
                            <h4>${r.companyName}</h4>
                            <h3>${r.title}</h3>
                        </div>
                    </div>
                    <div style="display: flex; flex-direction: column; align-items: flex-end; gap: 0.35rem;">
                        <span class="badge badge-fit">${r.matchPercentage}% Skill Fit</span>
                        <span class="badge ${r.isEligible ? 'badge-eligible' : 'badge-gap'}">
                            ${r.isEligible ? 'Eligible' : '1 Mandatory Gap'}
                        </span>
                    </div>
                </div>

                <div class="job-compensation">LKR ${r.stipend.toLocaleString()} <span style="font-size: 0.8rem; font-weight: 500; color: var(--text-muted);">/ month</span></div>

                <div class="job-meta-row">
                    <span class="job-meta-item">📍 ${r.location || 'Colombo'}</span>
                    <span class="job-meta-item">💼 ${r.mode}</span>
                    <span class="job-meta-item">⏳ Deadline: ${formattedDeadline}</span>
                </div>

                <div class="job-card-actions" onclick="event.stopPropagation()">
                    <button class="btn btn-secondary btn-sm" onclick="openRoadmapFor(${r.internshipId})">Roadmap Plan</button>
                    ${hasApplied ? `
                        <button class="btn btn-secondary btn-sm" disabled>Applied ✓</button>
                    ` : `
                        <button class="btn btn-primary btn-sm" onclick="applyToRole(${r.internshipId})" ${!r.isEligible ? 'title="Mandatory requirements not yet met"' : ''}>Apply Now</button>
                    `}
                </div>
            </div>
        `;
    }).join('');

    // Next Recommended Step Banner
    const nextStepCard = document.getElementById('overviewNextStepCard');
    if (nextStepCard) {
        const topGap = appState.recommendations.find(r => !r.isEligible);
        if (topGap) {
            nextStepCard.innerHTML = `
                <div style="background: white; border: 1px solid var(--border-card); border-radius: var(--radius-xl); padding: 1.5rem; display: flex; align-items: center; justify-content: space-between; gap: 1.5rem; flex-wrap: wrap;">
                    <div>
                        <h4 style="font-size: 1.05rem; font-weight: 700; color: var(--text-heading); margin-bottom: 0.25rem;">
                            Bridge mandatory criteria for ${topGap.title}
                        </h4>
                        <p style="font-size: 0.875rem; color: var(--text-muted);">
                            You have an <strong>${topGap.matchPercentage}% compatibility fit</strong>. Resolving your missing mandatory requirement unlocks immediate application eligibility.
                        </p>
                    </div>
                    <button class="btn btn-primary" onclick="openRoadmapFor(${topGap.internshipId})">Open Readiness Roadmap →</button>
                </div>
            `;
        } else {
            nextStepCard.innerHTML = `
                <div style="background: white; border: 1px solid var(--border-card); border-radius: var(--radius-xl); padding: 1.5rem; display: flex; align-items: center; justify-content: space-between;">
                    <div>
                        <h4 style="font-size: 1.05rem; font-weight: 700; color: var(--success); margin-bottom: 0.25rem;">
                            You are fully eligible for your top matches!
                        </h4>
                        <p style="font-size: 0.875rem; color: var(--text-muted);">All mandatory technical requirements are satisfied. Submit your application directly.</p>
                    </div>
                    <button class="btn btn-primary" onclick="switchNav('explore')">Explore All Roles →</button>
                </div>
            `;
        }
    }
}

// --- VIEW 2: Explore Internships (Two-Column Master-Detail) ---
function renderExplore() {
    const listPane = document.getElementById('exploreListingPane');
    const detailPane = document.getElementById('exploreDetailPane');
    if (!listPane || !detailPane) return;

    const filtered = appState.internships.filter(inItem => {
        const query = appState.searchQuery.toLowerCase();
        const matchesQuery = inItem.title.toLowerCase().includes(query) ||
                             inItem.companyName.toLowerCase().includes(query) ||
                             inItem.location.toLowerCase().includes(query);
        const matchesMode = (appState.modeFilter === 'ALL') || (inItem.mode === appState.modeFilter);
        return matchesQuery && matchesMode;
    });

    if (filtered.length === 0) {
        listPane.innerHTML = `<div style="padding: 2rem; text-align: center; color: var(--text-muted); background: white; border-radius: var(--radius-lg); border: 1px solid var(--border-light);">No internships matching search criteria.</div>`;
        detailPane.innerHTML = `<p style="color: var(--text-muted); text-align: center;">Select an internship from the directory to inspect technical expectations.</p>`;
        return;
    }

    if (!appState.selectedExploreId || !filtered.some(i => i.id === appState.selectedExploreId)) {
        appState.selectedExploreId = filtered[0].id;
    }

    // Render Left Listing
    listPane.innerHTML = filtered.map(inItem => {
        const isSelected = inItem.id === appState.selectedExploreId;
        const rec = appState.recommendations.find(r => r.internshipId === inItem.id);

        return `
            <div class="job-card" style="${isSelected ? 'border-color: var(--primary); box-shadow: 0 0 0 2px var(--primary-border);' : ''}" onclick="selectExploreInternship(${inItem.id})">
                <div class="job-card-top">
                    <div class="job-brand-group">
                        <div class="company-avatar">${(inItem.companyName || 'C').charAt(0)}</div>
                        <div class="job-headings">
                            <h4>${inItem.companyName}</h4>
                            <h3>${inItem.title}</h3>
                        </div>
                    </div>
                    ${rec ? `<span class="badge badge-fit">${rec.matchPercentage}% Fit</span>` : ''}
                </div>

                <div class="job-compensation">LKR ${inItem.stipend.toLocaleString()} <span style="font-size: 0.8rem; font-weight: 500; color: var(--text-muted);">/ mo</span></div>

                <div class="job-meta-row">
                    <span class="job-meta-item">📍 ${inItem.location}</span>
                    <span class="job-meta-item">💼 ${inItem.mode}</span>
                    <span class="job-meta-item">👥 ${inItem.capacity} Seats</span>
                </div>
            </div>
        `;
    }).join('');

    // Render Right Sticky Detail Pane
    const selected = appState.internships.find(i => i.id === appState.selectedExploreId);
    if (!selected) return;

    const student = appState.students.find(s => s.id === appState.activeStudentId);
    const rec = appState.recommendations.find(r => r.internshipId === selected.id);
    const hasApplied = appState.applications.some(a => a.internshipId === selected.id && a.status !== 'Withdrawn');

    detailPane.innerHTML = `
        <div style="display: flex; align-items: flex-start; justify-content: space-between; margin-bottom: 1.25rem;">
            <div>
                <h4 style="font-size: 0.9rem; font-weight: 600; color: var(--text-muted);">${selected.companyName}</h4>
                <h2 style="font-size: 1.5rem; font-weight: 800; color: var(--text-heading);">${selected.title}</h2>
            </div>
            <span class="badge ${selected.status === 'Open' ? 'badge-eligible' : 'badge-neutral'}">${selected.status}</span>
        </div>

        <div class="job-meta-row" style="margin-bottom: 1.5rem;">
            <span class="job-meta-item">📍 ${selected.location}</span>
            <span class="job-meta-item">💼 ${selected.mode}</span>
            <span class="job-meta-item">💵 LKR ${selected.stipend.toLocaleString()}/month</span>
            <span class="job-meta-item">👥 Capacity: ${selected.capacity} Positions</span>
            <span class="job-meta-item">⏳ Deadline: ${formatDeadline(selected.deadline)}</span>
        </div>

        <!-- Separate Fit and Eligibility Alert (Core Differentiator) -->
        ${rec ? `
            <div style="background: ${rec.isEligible ? 'var(--success-bg)' : 'var(--warning-bg)'}; border: 1px solid ${rec.isEligible ? 'var(--success-border)' : 'var(--warning-border)'}; border-radius: var(--radius-md); padding: 1rem 1.25rem; margin-bottom: 1.5rem;">
                <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 0.35rem;">
                    <span style="font-size: 0.85rem; font-weight: 700; color: ${rec.isEligible ? 'var(--success)' : 'var(--warning)'};">
                        ${rec.matchPercentage}% Compatibility Fit (${rec.fitNumerator}/${rec.fitDenominator} Score)
                    </span>
                    <span class="badge ${rec.isEligible ? 'badge-eligible' : 'badge-gap'}">
                        ${rec.isEligible ? 'Eligible to Apply' : 'Mandatory Gap'}
                    </span>
                </div>
                <p style="font-size: 0.825rem; color: var(--text-heading); margin: 0;">
                    ${rec.isEligible 
                        ? 'You satisfy all mandatory technical competencies required for this role.' 
                        : 'Notice: One or more mandatory requirements are not yet satisfied. Complete your learning roadmap to become eligible.'}
                </p>
            </div>
        ` : ''}

        <h3 style="font-size: 1rem; font-weight: 700; color: var(--text-heading); margin-bottom: 0.5rem;">Role Overview</h3>
        <p style="font-size: 0.9rem; color: var(--text-body); line-height: 1.6; margin-bottom: 1.5rem;">
            ${selected.description || 'Collaborate with engineering teams on building high-performance technical systems and infrastructure.'}
        </p>

        <h3 style="font-size: 1rem; font-weight: 700; color: var(--text-heading); margin-bottom: 0.75rem;">Technical Skill Expectations</h3>
        <div class="criteria-box">
            ${selected.requirements.map(req => {
                const declaredSkill = student && student.skills ? student.skills.find(s => s.skillId === req.skillId) : null;
                const studentLevel = declaredSkill ? declaredSkill.level : 0;
                const meetsReq = studentLevel >= req.requiredLevel;

                return `
                    <div class="criteria-row">
                        <div>
                            <span class="criteria-name">${req.skillName}</span>
                            <span style="font-size: 0.75rem; color: var(--text-muted); margin-left: 0.5rem;">
                                Level ${req.requiredLevel}+ ${req.mandatory ? '(MANDATORY)' : '(Preferred)'}
                            </span>
                        </div>
                        <div class="criteria-status">
                            <span style="font-size: 0.8rem; font-weight: 600; color: ${meetsReq ? 'var(--success)' : (req.mandatory ? 'var(--danger)' : 'var(--text-muted)')};">
                                You: Level ${studentLevel} ${meetsReq ? '✓' : (req.mandatory ? '✗' : '')}
                            </span>
                        </div>
                    </div>
                `;
            }).join('')}
        </div>

        <div style="display: flex; gap: 0.75rem; margin-top: 1.5rem;">
            <button class="btn btn-secondary" onclick="openRoadmapFor(${selected.id})">Open Learning Roadmap</button>
            ${hasApplied ? `
                <button class="btn btn-secondary" disabled>Application Submitted ✓</button>
            ` : `
                <button class="btn btn-primary" onclick="applyToRole(${selected.id})" ${rec && !rec.isEligible ? 'title="Mandatory requirements not yet met"' : ''}>
                    Apply for Position
                </button>
            `}
        </div>
    `;
}

function selectExploreInternship(id) {
    appState.selectedExploreId = id;
    renderExplore();
}

function setModeFilter(mode, chipEl) {
    appState.modeFilter = mode;
    document.querySelectorAll('.filter-chip').forEach(c => c.classList.remove('active'));
    if (chipEl) chipEl.classList.add('active');
    renderExplore();
}

function handleGlobalSearch(query) {
    appState.searchQuery = query;
    if (appState.activeView === 'explore') {
        renderExplore();
    }
}

function openExploreDetail(internshipId) {
    appState.selectedExploreId = internshipId;
    switchNav('explore');
}

// --- VIEW 3: Career Learning Roadmap (Topological Connected Milestones) ---
async function openRoadmapFor(internshipId) {
    appState.selectedRoadmapId = internshipId;
    switchNav('roadmap');
}

async function handleRoadmapRoleChange(internshipIdStr) {
    appState.selectedRoadmapId = parseInt(internshipIdStr, 10);
    await renderRoadmap();
}

async function renderRoadmap() {
    const roleSelect = document.getElementById('roadmapRoleSelect');
    const container = document.getElementById('roadmapContainer');
    if (!roleSelect || !container) return;

    roleSelect.innerHTML = appState.internships.map(i => `
        <option value="${i.id}" ${i.id === appState.selectedRoadmapId ? 'selected' : ''}>
            ${i.title} (${i.companyName})
        </option>
    `).join('');

    if (!appState.selectedRoadmapId && appState.internships.length > 0) {
        appState.selectedRoadmapId = appState.internships[0].id;
    }

    const selectedRole = appState.internships.find(i => i.id === appState.selectedRoadmapId);
    if (!selectedRole) return;

    try {
        const plan = await fetch(`/api/students/${appState.activeStudentId}/plan/${appState.selectedRoadmapId}`).then(r => r.json());

        if (!plan.tasks || plan.tasks.length === 0) {
            container.innerHTML = `
                <div style="background: white; border: 1px solid var(--border-card); border-radius: var(--radius-xl); padding: 3rem; text-align: center;">
                    <div style="width: 48px; height: 48px; border-radius: 50%; background: var(--success-bg); color: var(--success); display: flex; align-items: center; justify-content: center; margin: 0 auto 1rem auto; font-size: 1.5rem; font-weight: 700;">✓</div>
                    <h3 style="color: var(--text-heading); margin-bottom: 0.5rem;">Fully Prepared for ${selectedRole.title}!</h3>
                    <p style="color: var(--text-muted); max-width: 520px; margin: 0 auto 1.5rem auto;">
                        Your declared technical skill proficiencies satisfy all foundational and core prerequisites for this role.
                    </p>
                    <button class="btn btn-primary" onclick="applyToRole(${selectedRole.id})">Submit Application Now</button>
                </div>
            `;
            return;
        }

        // Group milestones by topological phase
        const phases = {};
        plan.tasks.forEach(t => {
            const phaseKey = t.stage || 0;
            if (!phases[phaseKey]) phases[phaseKey] = [];
            phases[phaseKey].push(t);
        });

        let timelineHtml = '';
        const sortedKeys = Object.keys(phases).sort((a, b) => parseInt(a) - parseInt(b));

        sortedKeys.forEach((key, idx) => {
            const tasksInPhase = phases[key];
            const phaseTitle = key == 0 ? 'Foundational Prerequisites' :
                               key == 1 ? 'Core Technical Competencies' : `Specialized Milestone Phase ${parseInt(key) + 1}`;

            timelineHtml += `
                <div class="roadmap-step">
                    <div class="step-node">${idx + 1}</div>
                    <div class="step-card">
                        <div style="display: flex; justify-content: space-between; align-items: center; margin-bottom: 0.75rem;">
                            <h3 style="font-size: 1.05rem; font-weight: 700; color: var(--text-heading);">Phase ${idx + 1}: ${phaseTitle}</h3>
                            <span class="badge badge-fit">${tasksInPhase.length} Step(s)</span>
                        </div>
                        <div style="display: flex; flex-direction: column; gap: 0.75rem;">
                            ${tasksInPhase.map(task => `
                                <div style="display: flex; align-items: center; justify-content: space-between; padding: 0.75rem; background: var(--bg-canvas); border-radius: var(--radius-md); border: 1px solid var(--border-light);">
                                    <div>
                                        <div style="font-weight: 700; color: var(--text-heading); font-size: 0.95rem;">${task.skillName}</div>
                                        <div style="font-size: 0.8rem; color: var(--text-muted); margin-top: 0.15rem;">
                                            Current: <strong>Level ${task.currentLevel}</strong> ➔ Target: <strong>Level ${task.targetLevel}</strong>
                                        </div>
                                    </div>
                                    <button class="btn btn-secondary btn-sm" onclick="levelUpSkill(${task.skillId}, ${task.targetLevel})">
                                        Mark as Level ${task.targetLevel} ✓
                                    </button>
                                </div>
                            `).join('')}
                        </div>
                    </div>
                </div>
            `;
        });

        container.innerHTML = `
            <div style="background: white; border: 1px solid var(--border-card); border-radius: var(--radius-xl); padding: 2rem;">
                <div style="margin-bottom: 1.5rem;">
                    <h3 style="font-size: 1.25rem; font-weight: 800; color: var(--text-heading);">${selectedRole.title} Career Pathway</h3>
                    <p style="font-size: 0.875rem; color: var(--text-muted);">
                        Generated using reverse prerequisite graph closure. Complete earlier phases to satisfy foundation dependencies.
                    </p>
                </div>
                <div class="roadmap-timeline">
                    ${timelineHtml}
                </div>
            </div>
        `;
    } catch (err) {
        console.error('Error generating roadmap:', err);
    }
}

async function levelUpSkill(skillId, newLevel) {
    await updateStudentSkill(skillId, newLevel);
    await renderRoadmap();
}

// --- VIEW 4: My Skills Matrix ---
function renderSkills() {
    const container = document.getElementById('skillsMatrixGrid');
    if (!container) return;

    const student = appState.students.find(s => s.id === appState.activeStudentId);
    if (!student) return;

    container.innerHTML = appState.skills.map(sk => {
        const declared = (student.skills || []).find(s => s.skillId === sk.id);
        const currentLevel = declared ? declared.level : 0;

        return `
            <div class="skill-matrix-item">
                <div>
                    <span style="font-size: 0.75rem; font-weight: 700; color: var(--primary); text-transform: uppercase;">${sk.category || 'General'}</span>
                    <h3 style="font-size: 1rem; font-weight: 700; color: var(--text-heading); margin-top: 0.15rem;">${sk.name}</h3>
                </div>

                <div>
                    <div style="font-size: 0.75rem; color: var(--text-muted); margin-bottom: 0.4rem;">Proficiency:</div>
                    <div class="segmented-control">
                        <button class="seg-btn ${currentLevel === 0 ? 'active' : ''}" onclick="updateStudentSkill(${sk.id}, 0)">0: Unset</button>
                        <button class="seg-btn ${currentLevel === 1 ? 'active' : ''}" onclick="updateStudentSkill(${sk.id}, 1)">1: Novice</button>
                        <button class="seg-btn ${currentLevel === 2 ? 'active' : ''}" onclick="updateStudentSkill(${sk.id}, 2)">2: Competent</button>
                        <button class="seg-btn ${currentLevel === 3 ? 'active' : ''}" onclick="updateStudentSkill(${sk.id}, 3)">3: Proficient</button>
                    </div>
                </div>
            </div>
        `;
    }).join('');
}

async function updateStudentSkill(skillId, level) {
    try {
        const res = await fetch(`/api/students/${appState.activeStudentId}/skills`, {
            method: 'PUT',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ skillId, level })
        });
        const data = await res.json();
        if (data.ok) {
            showToast('Skill level updated (Undo frame saved)');
            await refreshStudentContext();
        } else {
            showToast(data.error || 'Failed to update skill', 'error');
        }
    } catch (err) {
        showToast('Network error updating skill', 'error');
    }
}

async function handleUndoSkillEdit() {
    try {
        const res = await fetch(`/api/students/${appState.activeStudentId}/undo`, { method: 'POST' });
        const data = await res.json();
        if (data.ok) {
            showToast('Last skill modification undone');
            await refreshStudentContext();
        } else {
            showToast(data.error || data.message || 'No edits to undo', 'error');
        }
    } catch (err) {
        showToast('Error performing undo', 'error');
    }
}

// --- VIEW 5: My Applications ---
function renderApplications() {
    const tableBody = document.getElementById('applicationsTableBody');
    if (!tableBody) return;

    if (appState.applications.length === 0) {
        tableBody.innerHTML = `
            <tr>
                <td colspan="6" style="text-align: center; padding: 3rem; color: var(--text-muted);">
                    No applications submitted yet. Browse open opportunities to apply.
                </td>
            </tr>
        `;
        return;
    }

    tableBody.innerHTML = appState.applications.map(app => {
        let badgeClass = 'badge-neutral';
        if (app.status === 'Allocated') badgeClass = 'badge-eligible';
        if (app.status === 'Waitlisted') badgeClass = 'badge-gap';
        if (app.status === 'Submitted') badgeClass = 'badge-fit';

        return `
            <tr>
                <td style="font-weight: 700; color: var(--text-heading);">${app.internshipTitle}</td>
                <td>${app.companyName}</td>
                <td>#${app.submitSequence}</td>
                <td>${app.fitNumerator}/${app.fitDenominator}</td>
                <td><span class="badge ${badgeClass}">${app.status}</span></td>
                <td>
                    ${app.status !== 'Withdrawn' ? `
                        <button class="btn btn-secondary btn-sm" onclick="withdrawApplication(${app.id})">Withdraw</button>
                    ` : `<span style="color: var(--text-muted); font-size: 0.8rem;">Withdrawn</span>`}
                </td>
            </tr>
        `;
    }).join('');
}

async function applyToRole(internshipId) {
    try {
        const res = await fetch('/api/applications', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                studentId: appState.activeStudentId,
                internshipId: internshipId
            })
        });

        const data = await res.json();
        if (data.ok) {
            showToast('Application successfully submitted!');
            await refreshStudentContext();
        } else {
            showToast(`Application rejected: ${data.error || data.message}`, 'error');
        }
    } catch (err) {
        showToast('Network error submitting application', 'error');
    }
}

async function withdrawApplication(appId) {
    if (!confirm('Withdraw this application? If you are allocated, the top waitlisted candidate will automatically be promoted in real time.')) {
        return;
    }

    try {
        const res = await fetch(`/api/applications/${appId}/withdraw`, { method: 'POST' });
        const data = await res.json();
        if (data.ok) {
            showToast('Application withdrawn (Auto-promotion triggered if waitlisted)');
            await refreshStudentContext();
            await fetchInitialData();
        } else {
            showToast(data.error || 'Withdrawal failed', 'error');
        }
    } catch (err) {
        showToast('Network error withdrawing application', 'error');
    }
}

// --- VIEW 6: Administration Console ---
function renderAdmin() {
    const tableBody = document.getElementById('adminRoundsTableBody');
    if (!tableBody) return;

    tableBody.innerHTML = appState.internships.map(inItem => `
        <tr>
            <td style="font-weight: 700; color: var(--text-heading);">${inItem.title}</td>
            <td>${inItem.companyName}</td>
            <td>${inItem.capacity} seats</td>
            <td><span class="badge ${inItem.status === 'Open' ? 'badge-eligible' : 'badge-neutral'}">${inItem.status}</span></td>
            <td>
                <div style="display: flex; gap: 0.5rem;">
                    ${inItem.status === 'Open' ? `
                        <button class="btn btn-primary btn-sm" onclick="closeRound(${inItem.id})">Close Round</button>
                    ` : `
                        <button class="btn btn-secondary btn-sm" onclick="promoteWaitlist(${inItem.id})">Promote Waitlist</button>
                    `}
                    <button class="btn btn-secondary btn-sm" onclick="inspectWaitlist(${inItem.id})">View Queue</button>
                </div>
            </td>
        </tr>
    `).join('');
}

async function closeRound(internshipId) {
    if (!confirm('Close this application round? The C++ backend will execute deterministic integer ranking: top applicants will be allocated up to capacity, and remaining applicants will be placed into the ranked waiting list.')) {
        return;
    }

    try {
        const res = await fetch(`/api/internships/${internshipId}/close`, { method: 'POST' });
        const data = await res.json();
        if (data.ok) {
            showToast(data.message || 'Round closed and ranked successfully');
            await fetchInitialData();
            renderAdmin();
        } else {
            showToast(data.error || 'Failed to close round', 'error');
        }
    } catch (err) {
        showToast('Network error closing round', 'error');
    }
}

async function promoteWaitlist(internshipId) {
    try {
        const res = await fetch(`/api/internships/${internshipId}/promote`, { method: 'POST' });
        const data = await res.json();
        if (data.ok) {
            showToast(`Promoted candidate: ${data.studentName} (App #${data.promotedApplicationId})`);
            await fetchInitialData();
            renderAdmin();
        } else {
            showToast(data.error || 'No candidates to promote', 'error');
        }
    } catch (err) {
        showToast('Error promoting candidate', 'error');
    }
}

async function inspectWaitlist(internshipId) {
    try {
        const res = await fetch(`/api/internships/${internshipId}/waitlist`).then(r => r.json());
        if (!res || res.length === 0) {
            alert('Waiting list queue is currently empty for this opportunity.');
            return;
        }

        const listText = res.map(w => `Rank #${w.rank}: ${w.studentName} (Candidate ID: ${w.studentId}, App #${w.applicationId})`).join('\n');
        alert(`Ranked Waiting List Queue:\n\n${listText}`);
    } catch (err) {
        showToast('Error retrieving waitlist', 'error');
    }
}

async function saveCsvSnapshot() {
    try {
        const res = await fetch('/api/admin/save', { method: 'POST' }).then(r => r.json());
        if (res.ok) {
            showToast('Snapshot saved cleanly to CSV');
        } else {
            showToast(res.error || 'Failed to save snapshot', 'error');
        }
    } catch (err) {
        showToast('Error saving snapshot', 'error');
    }
}

async function reloadCsvSnapshot() {
    if (!confirm('Reload snapshot from disk? In-memory state will be replaced by CSV records.')) return;
    try {
        const res = await fetch('/api/admin/reload', { method: 'POST' }).then(r => r.json());
        if (res.ok) {
            showToast('Snapshot reloaded from disk');
            await fetchInitialData();
        } else {
            showToast(res.error || 'Failed to reload snapshot', 'error');
        }
    } catch (err) {
        showToast('Error reloading snapshot', 'error');
    }
}

// --- Modals & Forms ---
function openAddCandidateModal() {
    document.getElementById('addCandidateModal').classList.add('active');
}

function openAddCompanyModal() {
    document.getElementById('addCompanyModal').classList.add('active');
}

function openAddInternshipModal() {
    const select = document.getElementById('modalCompanySelect');
    if (select) {
        select.innerHTML = appState.companies.map(c => `<option value="${c.id}">${c.name}</option>`).join('');
    }
    document.getElementById('addInternshipModal').classList.add('active');
}

function closeModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) modal.classList.remove('active');
}

async function submitNewCandidate(e) {
    e.preventDefault();
    const name = document.getElementById('candidateNameInput').value;
    const email = document.getElementById('candidateEmailInput').value;

    try {
        const res = await fetch('/api/students', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ name, email })
        });
        const data = await res.json();
        if (data.ok) {
            showToast('Candidate profile registered');
            closeModal('addCandidateModal');
            document.getElementById('addCandidateForm').reset();
            await fetchInitialData();
            appState.activeStudentId = data.id;
            await refreshStudentContext();
        } else {
            showToast(data.error || 'Failed to register', 'error');
        }
    } catch (err) {
        showToast('Error creating candidate', 'error');
    }
}

async function submitNewCompany(e) {
    e.preventDefault();
    const name = document.getElementById('companyNameInput').value;
    const description = document.getElementById('companyDescInput').value;

    try {
        const res = await fetch('/api/companies', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ name, description })
        });
        const data = await res.json();
        if (data.ok) {
            showToast('Partner company registered');
            closeModal('addCompanyModal');
            document.getElementById('addCompanyForm').reset();
            await fetchInitialData();
        } else {
            showToast(data.error || 'Failed to add company', 'error');
        }
    } catch (err) {
        showToast('Error creating company', 'error');
    }
}

async function submitNewInternship(e) {
    e.preventDefault();
    const companyId = parseInt(document.getElementById('modalCompanySelect').value, 10);
    const title = document.getElementById('modalRoleTitle').value;
    const description = document.getElementById('modalRoleDesc').value;
    const location = document.getElementById('modalRoleLoc').value;
    const mode = document.getElementById('modalRoleMode').value;
    const stipend = parseInt(document.getElementById('modalRoleStipend').value, 10);
    const capacity = parseInt(document.getElementById('modalRoleCapacity').value, 10);

    try {
        const res = await fetch('/api/internships', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                companyId,
                title,
                description,
                location,
                mode,
                stipend,
                capacity,
                deadline: 20261231,
                requirements: [
                    { skillId: 1, requiredLevel: 2, weight: 5, mandatory: true }
                ]
            })
        });
        const data = await res.json();
        if (data.ok) {
            showToast('Internship published successfully');
            closeModal('addInternshipModal');
            document.getElementById('addInternshipForm').reset();
            await fetchInitialData();
        } else {
            showToast(data.error || 'Failed to publish role', 'error');
        }
    } catch (err) {
        showToast('Error publishing internship', 'error');
    }
}

function formatDeadline(dateNum) {
    if (!dateNum) return 'N/A';
    const s = dateNum.toString();
    if (s.length !== 8) return s;
    return `${s.substring(0, 4)}-${s.substring(4, 6)}-${s.substring(6, 8)}`;
}
