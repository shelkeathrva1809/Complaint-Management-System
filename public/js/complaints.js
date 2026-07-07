// ══════════════════════════════════════════════════════════
//  Complaints — Registration Form + Table + Filters
// ══════════════════════════════════════════════════════════

// ── Load complaints table ──
async function loadComplaints() {
    const el = document.getElementById('complaintsContent');
    if (!el) return;
    el.innerHTML = '<div style="text-align:center;padding:40px"><span class="spinner"></span></div>';
    const data = await api('/api/complaints');
    if (!data || !data.complaints) { el.innerHTML = '<p style="color:var(--text-muted)">No complaints found</p>'; return; }
    renderComplaintsTable(el, data.complaints, true);
}

// ── Load officer's assigned complaints ──
async function loadAssigned() {
    const el = document.getElementById('assignedContent');
    if (!el) return;
    el.innerHTML = '<div style="text-align:center;padding:40px"><span class="spinner"></span></div>';
    const data = await api('/api/complaints');
    if (!data || !data.complaints) { el.innerHTML = '<p style="color:var(--text-muted)">No assigned complaints</p>'; return; }
    renderComplaintsTable(el, data.complaints, false);
}

function renderComplaintsTable(container, complaints, showActions) {
    const user = getUser();
    let html = `
    <div class="table-controls">
        <input class="search-input" placeholder="🔍 Search complaints..." oninput="filterTable(this.value)">
        <select class="filter-select" onchange="filterStatus(this.value)">
            <option value="">All Status</option>
            <option value="Pending">Pending</option>
            <option value="Assigned">Assigned</option>
            <option value="In Progress">In Progress</option>
            <option value="Resolved">Resolved</option>
            <option value="Escalated">Escalated</option>
            <option value="Closed">Closed</option>
        </select>
        <select class="filter-select" onchange="filterType(this.value)">
            <option value="">All Types</option>
            <option value="Road">Road</option>
            <option value="Water">Water</option>
            <option value="Electricity">Electricity</option>
        </select>
    </div>
    <div style="overflow-x:auto">
    <table class="data-table" id="complaintsTable">
    <thead><tr>
        <th>ID</th><th>Tracking ID</th><th>Type</th><th>Description</th>
        <th>Severity</th><th>Status</th><th>Officer</th><th>Date</th>`;
    if (user && (user.role === 'officer' || user.role === 'admin')) html += '<th>Actions</th>';
    if (user && user.role === 'citizen') html += '<th>Actions</th>';
    html += '</tr></thead><tbody>';

    if (complaints.length === 0) {
        html += '<tr><td colspan="9" style="text-align:center;padding:40px;color:var(--text-muted)">No complaints found</td></tr>';
    } else {
        for (const c of complaints) {
            const sevClass = c.severity === 'Emergency' ? 'emergency' : c.severity === 'Normal' ? 'normal' : 'low';
            const statClass = c.status.toLowerCase().replace(' ', '-');
            const badgeClass = c.status === 'In Progress' ? 'progress' : statClass;
            html += `<tr data-status="${c.status}" data-type="${c.type}">
                <td><strong>#${c.id}</strong></td>
                <td style="font-size:0.75rem;color:var(--text-muted)">${c.trackingId}</td>
                <td>${c.type}</td>
                <td style="max-width:200px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap" title="${c.description}">${c.description}</td>
                <td><span class="badge badge-${sevClass}">${c.severity}</span></td>
                <td><span class="badge badge-${badgeClass}">${c.status}</span></td>
                <td>${c.officerName || '-'}</td>
                <td style="font-size:0.78rem">${c.createdAt ? c.createdAt.split(' ')[0] : '-'}</td>`;
            if (user && (user.role === 'officer' || user.role === 'admin')) {
                html += `<td>
                    <select class="filter-select" style="padding:4px 8px;font-size:0.75rem" onchange="updateStatus(${c.id}, this.value)">
                        <option value="">Change...</option>
                        <option value="In Progress">In Progress</option>
                        <option value="Resolved">Resolved</option>
                        <option value="Escalated">Escalated</option>
                    </select>
                </td>`;
            }
            if (user && user.role === 'citizen') {
                let actions = '';
                if (c.status === 'Resolved' && c.resolutionProof) {
                    actions = `<button class="btn btn-accent" style="padding:4px 8px;font-size:0.72rem;margin-right:4px" onclick="acceptResolution(${c.id})">Accept</button>
                               <button class="btn btn-outline" style="padding:4px 8px;font-size:0.72rem" onclick="rejectResolution(${c.id})">Reject</button>`;
                } else if (c.status === 'Closed' && c.rating === 0) {
                    actions = `<button class="btn btn-accent" style="padding:4px 8px;font-size:0.72rem" onclick="showRating(${c.id})">Rate ⭐</button>`;
                } else if (c.rating > 0) {
                    actions = '⭐'.repeat(c.rating);
                } else {
                    actions = '-';
                }
                html += `<td>${actions}</td>`;
            }
            html += '</tr>';
        }
    }
    html += '</tbody></table></div>';
    container.innerHTML = html;
}

// ── Table Filters ──
function filterTable(query) {
    query = query.toLowerCase();
    document.querySelectorAll('#complaintsTable tbody tr').forEach(row => {
        row.style.display = row.textContent.toLowerCase().includes(query) ? '' : 'none';
    });
}

function filterStatus(status) {
    document.querySelectorAll('#complaintsTable tbody tr').forEach(row => {
        if (!status) { row.style.display = ''; return; }
        row.style.display = row.dataset.status === status ? '' : 'none';
    });
}

function filterType(type) {
    document.querySelectorAll('#complaintsTable tbody tr').forEach(row => {
        if (!type) { row.style.display = ''; return; }
        row.style.display = row.dataset.type === type ? '' : 'none';
    });
}

// ── Status Update (Officer) ──
async function updateStatus(id, status) {
    if (!status) return;
    const data = await api(`/api/complaints/${id}/status`, {
        method: 'PATCH',
        body: JSON.stringify({ status })
    });
    if (data && data.success) {
        showToast('Status updated to: ' + status, 'success');
        loadComplaints();
        loadNotifCount();
    }
}

// ── Resolution Accept/Reject (Citizen) ──
async function acceptResolution(id) {
    const data = await api(`/api/complaints/${id}/accept`, { method: 'PATCH' });
    if (data && data.success) {
        showToast('Resolution accepted!', 'success');
        showRating(id);
        loadComplaints();
    }
}

async function rejectResolution(id) {
    if (!confirm('Reject this resolution? The complaint will go back to In Progress.')) return;
    const data = await api(`/api/complaints/${id}/reject`, { method: 'PATCH' });
    if (data && data.success) {
        showToast('Resolution rejected. Complaint reopened.', 'info');
        loadComplaints();
    }
}

// ── Rating ──
function showRating(complaintId) {
    const modal = document.createElement('div');
    modal.style.cssText = 'position:fixed;inset:0;background:rgba(0,0,0,0.6);z-index:999;display:flex;align-items:center;justify-content:center';
    modal.innerHTML = `
        <div class="card" style="max-width:380px;text-align:center;padding:32px">
            <h3 style="margin-bottom:16px;font-size:1.1rem">Rate Officer Service</h3>
            <div class="stars" style="justify-content:center;margin-bottom:16px" id="ratingStars">
                ${[1,2,3,4,5].map(i => `<span class="star" data-val="${i}" onclick="selectStar(${i})">★</span>`).join('')}
            </div>
            <textarea id="ratingFeedback" placeholder="Optional feedback..." 
                style="width:100%;padding:10px;background:var(--bg-input);border:1px solid var(--border);border-radius:8px;color:var(--text-primary);margin-bottom:16px;min-height:60px;resize:none"></textarea>
            <div style="display:flex;gap:8px">
                <button class="btn btn-outline" style="flex:1" onclick="this.closest('div[style*=fixed]').remove()">Cancel</button>
                <button class="btn btn-accent" style="flex:1" onclick="submitRating(${complaintId}, this)">Submit</button>
            </div>
        </div>`;
    document.body.appendChild(modal);
    window._selectedRating = 0;
}

function selectStar(val) {
    window._selectedRating = val;
    document.querySelectorAll('#ratingStars .star').forEach(s => {
        s.classList.toggle('active', parseInt(s.dataset.val) <= val);
    });
}

async function submitRating(id, btn) {
    if (!window._selectedRating) { showToast('Please select a rating', 'error'); return; }
    const feedback = document.getElementById('ratingFeedback')?.value || '';
    const data = await api(`/api/complaints/${id}/rate`, {
        method: 'POST',
        body: JSON.stringify({ rating: window._selectedRating, feedback })
    });
    if (data && data.success) {
        showToast('Thank you for your feedback! ⭐', 'success');
        btn.closest('div[style*=fixed]').remove();
        loadComplaints();
    }
}

// ── Register Complaint ──
async function submitComplaint(e) {
    e.preventDefault();
    const type = document.getElementById('compType').value;
    const description = document.getElementById('compDesc').value.trim();
    const location = document.getElementById('compLocation').value.trim();
    const severity = document.getElementById('compSeverity').value;

    // Validate
    const dv = validateDescription(description);
    if (!dv.valid) { showToast(dv.error, 'error'); return; }

    const btn = e.target.querySelector('button[type=submit]');
    btn.innerHTML = '<span class="spinner"></span> Submitting...';
    btn.disabled = true;

    const data = await api('/api/complaints', {
        method: 'POST',
        body: JSON.stringify({ type, description, location, severity })
    });

    if (data && data.success) {
        showToast('Complaint registered! ID: #' + data.complaint.id, 'success');
        e.target.reset();
        loadNotifCount();
    } else {
        showToast(data?.error || 'Failed to register', 'error');
    }
    btn.innerHTML = 'Submit Complaint';
    btn.disabled = false;
}
