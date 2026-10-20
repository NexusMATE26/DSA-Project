// InternPath - Enterprise Single Page Application Controller

const state = {
    currentStudentId: 1,
    students: [],
    skills: [],
    companies: [],
    internships: [],
    recommendations: [],
    applications: [],
    prerequisites: [],
    selectedInternshipForPlan: null,
    today: 20261020
};

// --- Initialization ---
document.addEventListener('DOMContentLoaded', async () => {
    setupNavigation();
    setupEventListeners();
    await loadInitialData();
});

// --- Toast System ---
function showToast(message, type = 'success') {
    const container = document.getElementById('toastContainer');
    if (!container) return;

    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;
    toast.innerHTML = `
        <span class="toast-icon">${type === 'success' ? '✓' : '⚠'}</span>
        <span class="toast-msg">${message}</span>
    `;

    container.appendChild(toast);
    setTimeout(() => toast.remove(), 3000);
}

// --- Navigation Tabs ---
function setupNavigation() {
    const tabs = document.querySelectorAll('.nav-tab');
    tabs.forEach(tab => {
        tab.addEventListener('click', () => {
            tabs.forEach(t => t.classList.remove('active'));
            tab.classList.add('active');

            const targetView = tab.getAttribute('data-view');
            switchView(targetView);
        });
    });
}

function switchView(viewName) {
    document.querySelectorAll('.view-section').forEach(view => {
        view.classList.remove('active');
    });

    const activeView = document.getElementById(`view-${viewName}`);
    if (activeView) {
        activeView.classList.add('active');
    }

    if (viewName === 'recommendations') renderRecommendations();
    if (viewName === 'explore') renderCatalogue();
    if (viewName === 'planner') renderLearningPlanView();
    if (viewName === 'applications') renderApplications();
    if (viewName === 'profile') renderProfile();
    if (viewName === 'admin') renderAdmin();
}

// --- Event Listeners ---
function setupEventListeners() {
    // Student Switcher
    const studentSelect = document.getElementById('studentSelect');
    if (studentSelect) {
        studentSelect.addEventListener('change', async (e) => {
            state.currentStudentId = parseInt(e.target.value, 10);
            await refreshActiveStudentContext();
            showToast(`Switched active profile`, 'success');
        });
    }

    // Modal triggers
    const openAddStudentModalBtn = document.getElementById('openAddStudentModal');
    if (openAddStudentModalBtn) {
        openAddStudentModalBtn.addEventListener('click', () => {
            document.getElementById('addStudentModal').classList.add('active');
        });
    }

    const openAddCompanyModalBtn = document.getElementById('openAddCompanyModal');
    if (openAddCompanyModalBtn) {
        openAddCompanyModalBtn.addEventListener('click', () => {
            document.getElementById('addCompanyModal').classList.add('active');
        });
    }

    const openAddInternshipModalBtn = document.getElementById('openAddInternshipModal');
    if (openAddInternshipModalBtn) {
        openAddInternshipModalBtn.addEventListener('click', () => {
            populateCompanySelectInModal();
            document.getElementById('addInternshipModal').classList.add('active');
        });
    }

    // Close Modals
    document.querySelectorAll('.modal-close, .modal-cancel').forEach(btn => {
        btn.addEventListener('click', () => {
            document.querySelectorAll('.modal-overlay').forEach(m => m.classList.remove('active'));
        });
    });

    // Form Submissions
    setupFormHandlers();
}

// --- Data Fetching ---
async function loadInitialData() {
    try {
        const [healthRes, studentsRes, internshipsRes, companiesRes, skillsRes] = await Promise.all([
            fetch('/api/health').then(r => r.json()),
            fetch('/api/students').then(r => r.json()),
            fetch('/api/internships').then(r => r.json()),
            fetch('/api/companies').then(r => r.json()),
            fetch('/api/skills').then(r => r.json())
        ]);

        if (healthRes && healthRes.today) state.today = healthRes.today;
        state.students = studentsRes || [];
        state.internships = internshipsRes || [];
        state.companies = companiesRes || [];
        state.skills = (skillsRes && skillsRes.skills) ? skillsRes.skills : [];
        state.prerequisites = (skillsRes && skillsRes.prerequisites) ? skillsRes.prerequisites : [];

        // Set default student if exists
        if (state.students.length > 0 && !state.students.find(s => s.id === state.currentStudentId)) {
            state.currentStudentId = state.students[0].id;
        }

        renderStudentDropdown();
        await refreshActiveStudentContext();
        renderCatalogue();
    } catch (err) {
        console.error('Failed to load initial data:', err);
        showToast('Error connecting to backend server', 'error');
    }
}

async function refreshActiveStudentContext() {
    if (!state.currentStudentId) return;

    try {
        const [recsRes, appsRes, studentRes] = await Promise.all([
            fetch(`/api/students/${state.currentStudentId}/recommendations`).then(r => r.json()),
            fetch(`/api/students/${state.currentStudentId}/applications`).then(r => r.json()),
            fetch(`/api/students/${state.currentStudentId}`).then(r => r.json())
        ]);

        state.recommendations = recsRes || [];
        state.applications = appsRes || [];

        // Update current student data
        const idx = state.students.findIndex(s => s.id === state.currentStudentId);
        if (idx !== -1 && studentRes) {
            state.students[idx] = studentRes;
        }

        renderStats();
        renderRecommendations();
        renderApplications();
        renderProfile();
    } catch (err) {
        console.error('Error refreshing student context:', err);
    }
}

function renderStudentDropdown() {
    const studentSelect = document.getElementById('studentSelect');
    if (!studentSelect) return;

    studentSelect.innerHTML = state.students.map(s => `
        <option value="${s.id}" ${s.id === state.currentStudentId ? 'selected' : ''}>
            ${s.name} (ID: ${s.id})
        </option>
    `).join('');
}

function renderStats() {
    const totalOpenings = state.internships.filter(i => i.status === 'Open').length;
    const eligibleMatches = state.recommendations.filter(r => r.isEligible).length;
    const topMatchPct = state.recommendations.length > 0 ? state.recommendations[0].matchPercentage : 0;
    const myAppsCount = state.applications.length;

    const elTotal = document.getElementById('statTotalOpenings');
    const elEligible = document.getElementById('statEligibleMatches');
    const elTopPct = document.getElementById('statTopMatch');
    const elMyApps = document.getElementById('statActiveApplications');

    if (elTotal) elTotal.textContent = totalOpenings;
    if (elEligible) elEligible.textContent = eligibleMatches;
    if (elTopPct) elTopPct.textContent = `${topMatchPct}%`;
    if (elMyApps) elMyApps.textContent = myAppsCount;
}

// --- View 1: Matches & Recommendations ---
function renderRecommendations() {
    const container = document.getElementById('recommendationsList');
    if (!container) return;

    if (state.recommendations.length === 0) {
        container.innerHTML = `
            <div class="empty-state">
                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                    <circle cx="12" cy="12" r="10"></circle>
                    <line x1="8" y1="12" x2="16" y2="12"></line>
                </svg>
                <h3>No Recommendations Available</h3>
                <p>Complete your profile skills to unlock personalized internship matches.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = state.recommendations.map(r => {
        const hasApplied = state.applications.some(a => a.internshipId === r.internshipId && a.status !== 'Withdrawn');
        const formattedDeadline = formatDeadline(r.deadline);

        return `
            <div class="glass-card">
                <div>
                    <div class="card-top">
                        <div class="company-badge">
                            <div class="company-logo-placeholder">${r.companyName.charAt(0)}</div>
                            <div class="company-details">
                                <h4>${r.companyName}</h4>
                                <h3>${r.title}</h3>
                            </div>
                        </div>
                        <span class="badge ${r.isEligible ? 'badge-success' : 'badge-danger'}">
                            ${r.isEligible ? 'Eligible' : 'Skill Gap'}
                        </span>
                    </div>

                    <div class="fit-gauge">
                        <div class="fit-header">
                            <span class="fit-title">Compatibility Match</span>
                            <span class="fit-score">${r.matchPercentage}% <small>(${r.fitNumerator}/${r.fitDenominator})</small></span>
                        </div>
                        <div class="progress-track">
                            <div class="progress-fill" style="width: ${r.matchPercentage}%"></div>
                        </div>
                    </div>

                    <div class="card-meta">
                        <span class="meta-item">📍 ${r.location || 'Colombo'}</span>
                        <span class="meta-item">💼 ${r.mode}</span>
                        <span class="meta-item">💵 LKR ${r.stipend.toLocaleString()}/mo</span>
                        <span class="meta-item">⏳ Deadline: ${formattedDeadline}</span>
                    </div>
                </div>

                <div class="card-actions">
                    <button class="btn btn-secondary btn-sm" onclick="openLearningPlanFor(${r.internshipId})">
                        <span>Roadmap & Plan</span>
                    </button>
                    ${hasApplied ? `
                        <button class="btn btn-secondary btn-sm" disabled>
                            <span>Applied ✓</span>
                        </button>
                    ` : `
                        <button class="btn btn-primary btn-sm" 
                                onclick="applyNow(${r.internshipId})"
                                ${!r.isEligible ? 'title="Mandatory requirements not yet met"' : ''}>
                            <span>Apply Now</span>
                        </button>
                    `}
                </div>
            </div>
        `;
    }).join('');
}

// --- View 2: Explore Catalogue ---
function renderCatalogue() {
    const container = document.getElementById('catalogueList');
    if (!container) return;

    const searchTerm = (document.getElementById('catalogueSearch')?.value || '').toLowerCase();
    const modeFilter = document.getElementById('catalogueModeFilter')?.value || 'ALL';

    const filtered = state.internships.filter(inItem => {
        const matchesSearch = inItem.title.toLowerCase().includes(searchTerm) ||
                              inItem.companyName.toLowerCase().includes(searchTerm) ||
                              inItem.location.toLowerCase().includes(searchTerm);
        const matchesMode = (modeFilter === 'ALL') || (inItem.mode === modeFilter);
        return matchesSearch && matchesMode;
    });

    if (filtered.length === 0) {
        container.innerHTML = `
            <div class="empty-state">
                <h3>No Matching Internships</h3>
                <p>Try refining your search terms or filters.</p>
            </div>
        `;
        return;
    }

    container.innerHTML = filtered.map(inItem => {
        const hasApplied = state.applications.some(a => a.internshipId === inItem.id && a.status !== 'Withdrawn');
        const formattedDeadline = formatDeadline(inItem.deadline);

        return `
            <div class="glass-card">
                <div>
                    <div class="card-top">
                        <div class="company-badge">
                            <div class="company-logo-placeholder">${inItem.companyName.charAt(0)}</div>
                            <div class="company-details">
                                <h4>${inItem.companyName}</h4>
                                <h3>${inItem.title}</h3>
                            </div>
                        </div>
                        <span class="badge ${inItem.status === 'Open' ? 'badge-primary' : 'badge-neutral'}">
                            ${inItem.status}
                        </span>
                    </div>

                    <p style="font-size: 0.875rem; color: var(--text-secondary); margin-bottom: 1rem;">
                        ${inItem.description || 'Join our engineering team and build industry-grade software solutions.'}
                    </p>

                    <div style="margin-bottom: 1rem;">
                        <span style="font-size: 0.75rem; font-weight: 700; color: var(--text-muted); text-transform: uppercase;">
                            Required Skills:
                        </span>
                        <div style="display: flex; flex-wrap: wrap; gap: 0.4rem; margin-top: 0.4rem;">
                            ${inItem.requirements.map(req => `
                                <span class="badge ${req.mandatory ? 'badge-primary' : 'badge-neutral'}" style="font-size: 0.7rem;">
                                    ${req.skillName} (Lvl ${req.requiredLevel}${req.mandatory ? '*' : ''})
                                </span>
                            `).join('')}
                        </div>
                    </div>

                    <div class="card-meta">
                        <span class="meta-item">📍 ${inItem.location}</span>
                        <span class="meta-item">💼 ${inItem.mode}</span>
                        <span class="meta-item">💵 LKR ${inItem.stipend.toLocaleString()}/mo</span>
                        <span class="meta-item">👥 Capacity: ${inItem.capacity}</span>
                        <span class="meta-item">⏳ Deadline: ${formattedDeadline}</span>
                    </div>
                </div>

                <div class="card-actions">
                    <button class="btn btn-secondary btn-sm" onclick="openLearningPlanFor(${inItem.id})">
                        <span>Learning Plan</span>
                    </button>
                    ${hasApplied ? `
                        <button class="btn btn-secondary btn-sm" disabled>
                            <span>Applied ✓</span>
                        </button>
                    ` : `
                        <button class="btn btn-primary btn-sm" 
                                onclick="applyNow(${inItem.id})"
                                ${inItem.status !== 'Open' ? 'disabled' : ''}>
                            <span>Apply</span>
                        </button>
                    `}
                </div>
            </div>
        `;
    }).join('');
}

// --- View 3: Learning Plan & Skill Gap ---
async function openLearningPlanFor(internshipId) {
    state.selectedInternshipForPlan = internshipId;
    const tab = document.querySelector('.nav-tab[data-view="planner"]');
    if (tab) tab.click();
}

async function renderLearningPlanView() {
    const selector = document.getElementById('plannerInternshipSelect');
    const container = document.getElementById('roadmapContent');
    if (!selector || !container) return;

    // Populate dropdown
    selector.innerHTML = state.internships.map(inItem => `
        <option value="${inItem.id}" ${inItem.id === state.selectedInternshipForPlan ? 'selected' : ''}>
            ${inItem.title} - ${inItem.companyName}
        </option>
    `).join('');

    if (!state.selectedInternshipForPlan && state.internships.length > 0) {
        state.selectedInternshipForPlan = state.internships[0].id;
    }

    if (!state.selectedInternshipForPlan) {
        container.innerHTML = `<div class="empty-state"><h3>Select an internship to view its learning roadmap</h3></div>`;
        return;
    }

    try {
        const plan = await fetch(`/api/students/${state.currentStudentId}/plan/${state.selectedInternshipForPlan}`).then(r => r.json());
        const selectedInternship = state.internships.find(i => i.id === state.selectedInternshipForPlan);

        if (!plan.tasks || plan.tasks.length === 0) {
            container.innerHTML = `
                <div class="roadmap-header">
                    <div>
                        <h2>Ready to Apply!</h2>
                        <p style="color: var(--success); font-weight: 600;">You have fulfilled all required skill proficiencies for ${selectedInternship ? selectedInternship.title : 'this role'}.</p>
                    </div>
                    <button class="btn btn-primary" onclick="applyNow(${state.selectedInternshipForPlan})">Apply Now</button>
                </div>
            `;
            return;
        }

        // Group tasks by topological stage
        const stages = {};
        plan.tasks.forEach(task => {
            const stageKey = task.stage || 0;
            if (!stages[stageKey]) stages[stageKey] = [];
            stages[stageKey].push(task);
        });

        let stagesHtml = '';
        Object.keys(stages).sort((a, b) => parseInt(a) - parseInt(b)).forEach(stageNum => {
            const tasksInStage = stages[stageNum];
            const stageLabel = stageNum == 0 ? 'Phase 1: Foundational Prerequisites' :
                               stageNum == 1 ? 'Phase 2: Core Competencies' : `Phase ${parseInt(stageNum) + 1}: Target Specializations`;

            stagesHtml += `
                <div class="stage-group">
                    <div class="stage-title">
                        <span>✦</span> ${stageLabel}
                    </div>
                    <div class="stage-tasks">
                        ${tasksInStage.map(t => `
                            <div class="task-card">
                                <div class="task-top">
                                    <div class="task-name">${t.skillName}</div>
                                    <div class="level-progression">
                                        <span class="level-pill level-current">Current: Lvl ${t.currentLevel}</span>
                                        <span>➔</span>
                                        <span class="level-pill level-target">Target: Lvl ${t.targetLevel}</span>
                                    </div>
                                </div>
                                <div style="display: flex; justify-content: space-between; align-items: center; margin-top: 0.5rem;">
                                    <span style="font-size: 0.75rem; color: var(--text-muted);">Proficiency Gap: +${t.targetLevel - t.currentLevel} Level(s)</span>
                                    <button class="btn btn-accent btn-sm" onclick="boostSkill(${t.skillId}, ${t.targetLevel})">
                                        Level Up
                                    </button>
                                </div>
                            </div>
                        `).join('')}
                    </div>
                </div>
            `;
        });

        container.innerHTML = `
            <div class="roadmap-header">
                <div>
                    <h2>${selectedInternship ? selectedInternship.title : 'Target Internship'} Roadmap</h2>
                    <p style="color: var(--text-secondary);">Topologically ordered learning path generated via reverse prerequisite propagation.</p>
                </div>
                <button class="btn btn-primary" onclick="applyNow(${state.selectedInternshipForPlan})">Submit Application</button>
            </div>
            ${stagesHtml}
        `;
    } catch (err) {
        console.error('Error generating learning plan:', err);
        container.innerHTML = `<div class="empty-state"><h3>Unable to compute learning roadmap</h3></div>`;
    }
}

// --- View 4: My Applications ---
function renderApplications() {
    const tableBody = document.getElementById('applicationsTableBody');
    if (!tableBody) return;

    if (state.applications.length === 0) {
        tableBody.innerHTML = `
            <tr>
                <td colspan="6" style="text-align: center; padding: 3rem; color: var(--text-muted);">
                    No applications submitted yet. Browse open opportunities and apply!
                </td>
            </tr>
        `;
        return;
    }

    tableBody.innerHTML = state.applications.map(app => {
        let statusBadgeClass = 'badge-neutral';
        if (app.status === 'Allocated') statusBadgeClass = 'badge-success';
        if (app.status === 'Waitlisted') statusBadgeClass = 'badge-warning';
        if (app.status === 'Submitted') statusBadgeClass = 'badge-primary';
        if (app.status === 'Withdrawn') statusBadgeClass = 'badge-neutral';

        return `
            <tr>
                <td class="primary-cell">${app.internshipTitle}</td>
                <td>${app.companyName}</td>
                <td>#${app.submitSequence}</td>
                <td>${app.fitNumerator}/${app.fitDenominator}</td>
                <td>
                    <span class="badge ${statusBadgeClass}">${app.status}</span>
                </td>
                <td>
                    ${app.status !== 'Withdrawn' ? `
                        <button class="btn btn-danger btn-sm" onclick="withdrawApp(${app.id})">
                            Withdraw
                        </button>
                    ` : `<span style="color: var(--text-muted); font-size: 0.8rem;">Withdrawn</span>`}
                </td>
            </tr>
        `;
    }).join('');
}

// --- View 5: Profile & Skills ---
function renderProfile() {
    const currentStudent = state.students.find(s => s.id === state.currentStudentId);
    if (!currentStudent) return;

    const nameInput = document.getElementById('profileName');
    const emailInput = document.getElementById('profileEmail');
    if (nameInput) nameInput.value = currentStudent.name || '';
    if (emailInput) emailInput.value = currentStudent.email || '';

    // Skills Matrix
    const skillsContainer = document.getElementById('profileSkillsMatrix');
    if (!skillsContainer) return;

    skillsContainer.innerHTML = state.skills.map(sk => {
        const declaredSkill = (currentStudent.skills || []).find(s => s.skillId === sk.id);
        const currentLevel = declaredSkill ? declaredSkill.level : 0;

        return `
            <div class="skill-row">
                <div class="skill-info">
                    <h4>${sk.name}</h4>
                    <span>${sk.category || 'General'}</span>
                </div>
                <div class="level-selector">
                    ${[0, 1, 2, 3].map(lvl => `
                        <button class="level-btn ${currentLevel === lvl ? 'active' : ''}" 
                                onclick="setStudentSkill(${sk.id}, ${lvl})">
                            ${lvl}
                        </button>
                    `).join('')}
                </div>
            </div>
        `;
    }).join('');
}

// --- View 6: Admin Console ---
function renderAdmin() {
    renderAdminInternships();
}

function renderAdminInternships() {
    const container = document.getElementById('adminInternshipRows');
    if (!container) return;

    container.innerHTML = state.internships.map(inItem => `
        <tr>
            <td class="primary-cell">${inItem.title}</td>
            <td>${inItem.companyName}</td>
            <td>${inItem.capacity}</td>
            <td><span class="badge ${inItem.status === 'Open' ? 'badge-primary' : 'badge-neutral'}">${inItem.status}</span></td>
            <td>
                <div style="display: flex; gap: 0.5rem;">
                    ${inItem.status === 'Open' ? `
                        <button class="btn btn-secondary btn-sm" onclick="closeRound(${inItem.id})">
                            Close Round
                        </button>
                    ` : `
                        <button class="btn btn-accent btn-sm" onclick="promoteCandidate(${inItem.id})">
                            Promote Waitlist
                        </button>
                    `}
                    <button class="btn btn-secondary btn-sm" onclick="viewWaitlist(${inItem.id})">
                        Waitlist Queue
                    </button>
                </div>
            </td>
        </tr>
    `).join('');
}

// --- Operations & API Calls ---
async function applyNow(internshipId) {
    try {
        const res = await fetch('/api/applications', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                studentId: state.currentStudentId,
                internshipId: internshipId
            })
        });

        const data = await res.json();
        if (data.ok) {
            showToast('Application submitted successfully!', 'success');
            await refreshActiveStudentContext();
        } else {
            showToast(`Application error: ${data.error || data.message}`, 'error');
        }
    } catch (err) {
        showToast('Network error submitting application', 'error');
    }
}

async function withdrawApp(appId) {
    if (!confirm('Are you sure you want to withdraw this application? If allocated, the next candidate on the waiting list will automatically be promoted.')) {
        return;
    }

    try {
        const res = await fetch(`/api/applications/${appId}/withdraw`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' }
        });

        const data = await res.json();
        if (data.ok) {
            showToast('Application withdrawn successfully. (Auto-promotion triggered if waitlisted candidates existed)', 'success');
            await refreshActiveStudentContext();
            await loadInitialData();
        } else {
            showToast(`Error withdrawing: ${data.error || data.message}`, 'error');
        }
    } catch (err) {
        showToast('Network error withdrawing application', 'error');
    }
}

async function setStudentSkill(skillId, level) {
    try {
        const res = await fetch(`/api/students/${state.currentStudentId}/skills`, {
            method: 'PUT',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ skillId, level })
        });

        const data = await res.json();
        if (data.ok) {
            showToast('Proficiency updated', 'success');
            await refreshActiveStudentContext();
        } else {
            showToast(`Failed to update skill: ${data.error || data.message}`, 'error');
        }
    } catch (err) {
        showToast('Network error updating skill', 'error');
    }
}

async function undoProfileChange() {
    try {
        const res = await fetch(`/api/students/${state.currentStudentId}/undo`, {
            method: 'POST'
        });

        const data = await res.json();
        if (data.ok) {
            showToast('Profile change undone!', 'success');
            await refreshActiveStudentContext();
        } else {
            showToast(data.error || data.message || 'No profile edits left to undo', 'error');
        }
    } catch (err) {
        showToast('Network error performing undo', 'error');
    }
}

async function boostSkill(skillId, targetLevel) {
    await setStudentSkill(skillId, targetLevel);
    await renderLearningPlanView();
}

async function closeRound(internshipId) {
    if (!confirm('Close this round? Candidates will be deterministically ranked by integer fit. Top candidates will be allocated up to capacity, and remaining candidates will be placed into the ranked waiting list.')) {
        return;
    }

    try {
        const res = await fetch(`/api/internships/${internshipId}/close`, { method: 'POST' });
        const data = await res.json();
        if (data.ok) {
            showToast(data.message || 'Round closed successfully!', 'success');
            await loadInitialData();
            renderAdmin();
        } else {
            showToast(data.error || data.message, 'error');
        }
    } catch (err) {
        showToast('Failed to close round', 'error');
    }
}

async function promoteCandidate(internshipId) {
    try {
        const res = await fetch(`/api/internships/${internshipId}/promote`, { method: 'POST' });
        const data = await res.json();
        if (data.ok) {
            showToast(`Promoted candidate: ${data.studentName} (Student ID: ${data.studentId})`, 'success');
            await loadInitialData();
            renderAdmin();
        } else {
            showToast(data.error || data.message, 'error');
        }
    } catch (err) {
        showToast('Error promoting candidate', 'error');
    }
}

async function viewWaitlist(internshipId) {
    try {
        const res = await fetch(`/api/internships/${internshipId}/waitlist`).then(r => r.json());
        if (!res || res.length === 0) {
            alert('Waiting list is currently empty for this internship.');
            return;
        }

        const listText = res.map(w => `Rank #${w.rank}: ${w.studentName} (App ID: ${w.applicationId})`).join('\n');
        alert(`Ranked Waiting List Queue:\n\n${listText}`);
    } catch (err) {
        showToast('Error fetching waiting list', 'error');
    }
}

async function saveCsvSnapshot() {
    try {
        const res = await fetch('/api/admin/save', { method: 'POST' }).then(r => r.json());
        if (res.ok) {
            showToast('CSV snapshot saved cleanly to disk!', 'success');
        } else {
            showToast(res.error || 'Failed to save snapshot', 'error');
        }
    } catch (err) {
        showToast('Network error saving CSV snapshot', 'error');
    }
}

async function reloadCsvSnapshot() {
    if (!confirm('Reload dataset from CSV? Any unsaved memory state will be replaced by disk snapshot.')) return;
    try {
        const res = await fetch('/api/admin/reload', { method: 'POST' }).then(r => r.json());
        if (res.ok) {
            showToast('Snapshot reloaded from disk!', 'success');
            await loadInitialData();
        } else {
            showToast(res.error || 'Failed to reload snapshot', 'error');
        }
    } catch (err) {
        showToast('Network error reloading snapshot', 'error');
    }
}

// --- Form Handlers ---
function setupFormHandlers() {
    // Add Student
    const addStudentForm = document.getElementById('addStudentForm');
    if (addStudentForm) {
        addStudentForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            const name = document.getElementById('newStudentName').value;
            const email = document.getElementById('newStudentEmail').value;

            try {
                const res = await fetch('/api/students', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ name, email })
                });
                const data = await res.json();
                if (data.ok) {
                    showToast('Student registered successfully!', 'success');
                    document.getElementById('addStudentModal').classList.remove('active');
                    addStudentForm.reset();
                    await loadInitialData();
                    state.currentStudentId = data.id;
                    renderStudentDropdown();
                    await refreshActiveStudentContext();
                } else {
                    showToast(data.error || 'Failed to register student', 'error');
                }
            } catch (err) {
                showToast('Network error registering student', 'error');
            }
        });
    }

    // Add Company
    const addCompanyForm = document.getElementById('addCompanyForm');
    if (addCompanyForm) {
        addCompanyForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            const name = document.getElementById('newCompanyName').value;
            const description = document.getElementById('newCompanyDesc').value;

            try {
                const res = await fetch('/api/companies', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ name, description })
                });
                const data = await res.json();
                if (data.ok) {
                    showToast('Partner company added!', 'success');
                    document.getElementById('addCompanyModal').classList.remove('active');
                    addCompanyForm.reset();
                    await loadInitialData();
                } else {
                    showToast(data.error || 'Failed to add company', 'error');
                }
            } catch (err) {
                showToast('Network error adding company', 'error');
            }
        });
    }

    // Add Internship
    const addInternshipForm = document.getElementById('addInternshipForm');
    if (addInternshipForm) {
        addInternshipForm.addEventListener('submit', async (e) => {
            e.preventDefault();
            const companyId = parseInt(document.getElementById('newInternshipCompany').value, 10);
            const title = document.getElementById('newInternshipTitle').value;
            const description = document.getElementById('newInternshipDesc').value;
            const location = document.getElementById('newInternshipLocation').value;
            const mode = document.getElementById('newInternshipMode').value;
            const stipend = parseInt(document.getElementById('newInternshipStipend').value, 10);
            const capacity = parseInt(document.getElementById('newInternshipCapacity').value, 10);
            const deadline = parseInt(document.getElementById('newInternshipDeadline').value.replace(/-/g, ''), 10);

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
                        deadline,
                        requirements: [
                            { skillId: 1, requiredLevel: 2, weight: 5, mandatory: true }
                        ]
                    })
                });
                const data = await res.json();
                if (data.ok) {
                    showToast('Internship opportunity published!', 'success');
                    document.getElementById('addInternshipModal').classList.remove('active');
                    addInternshipForm.reset();
                    await loadInitialData();
                } else {
                    showToast(data.error || 'Failed to publish internship', 'error');
                }
            } catch (err) {
                showToast('Network error publishing internship', 'error');
            }
        });
    }

    // Update Profile Name/Email
    const saveProfileBtn = document.getElementById('saveProfileBtn');
    if (saveProfileBtn) {
        saveProfileBtn.addEventListener('click', async () => {
            const name = document.getElementById('profileName').value;
            const email = document.getElementById('profileEmail').value;

            try {
                const res = await fetch(`/api/students/${state.currentStudentId}`, {
                    method: 'PUT',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ name, email })
                });
                const data = await res.json();
                if (data.ok) {
                    showToast('Personal info updated', 'success');
                    await loadInitialData();
                } else {
                    showToast(data.error || 'Failed to update info', 'error');
                }
            } catch (err) {
                showToast('Error saving profile', 'error');
            }
        });
    }

    // Filter controls
    const catalogueSearch = document.getElementById('catalogueSearch');
    if (catalogueSearch) catalogueSearch.addEventListener('input', renderCatalogue);

    const catalogueModeFilter = document.getElementById('catalogueModeFilter');
    if (catalogueModeFilter) catalogueModeFilter.addEventListener('change', renderCatalogue);

    const plannerSelect = document.getElementById('plannerInternshipSelect');
    if (plannerSelect) {
        plannerSelect.addEventListener('change', (e) => {
            state.selectedInternshipForPlan = parseInt(e.target.value, 10);
            renderLearningPlanView();
        });
    }
}

function populateCompanySelectInModal() {
    const select = document.getElementById('newInternshipCompany');
    if (!select) return;
    select.innerHTML = state.companies.map(c => `
        <option value="${c.id}">${c.name}</option>
    `).join('');
}

function formatDeadline(dateNum) {
    if (!dateNum) return 'N/A';
    const s = dateNum.toString();
    if (s.length !== 8) return s;
    return `${s.substring(0, 4)}-${s.substring(4, 6)}-${s.substring(6, 8)}`;
}
