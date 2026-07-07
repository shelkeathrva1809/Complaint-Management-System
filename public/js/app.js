// ══════════════════════════════════════════════════════════
//  App Core — Auth, API, Navigation, Toasts
// ══════════════════════════════════════════════════════════

const API = '';
let currentUser = null;

// ── Auth helpers ──
function getToken() { return localStorage.getItem('spcms_token'); }
function getUser() {
    const u = localStorage.getItem('spcms_user');
    return u ? JSON.parse(u) : null;
}
function logout() {
    localStorage.removeItem('spcms_token');
    localStorage.removeItem('spcms_user');
    window.location.href = '/index.html';
}
function requireAuth() {
    if (!getToken()) { window.location.href = '/index.html'; return false; }
    return true;
}

// ── API helper ──
async function api(path, opts = {}) {
    const headers = { 'Content-Type': 'application/json' };
    const token = getToken();
    if (token) headers['Authorization'] = 'Bearer ' + token;
    try {
        const r = await fetch(API + path, { ...opts, headers: { ...headers, ...(opts.headers || {}) } });
        if (r.status === 401) { logout(); return null; }
        return await r.json();
    } catch (e) {
        showToast('Connection error', 'error');
        return null;
    }
}

// ── Toast Messages ──
function showToast(msg, type = 'info') {
    let container = document.getElementById('toastContainer');
    if (!container) {
        container = document.createElement('div');
        container.id = 'toastContainer';
        container.className = 'toast-container';
        document.body.appendChild(container);
    }
    const toast = document.createElement('div');
    toast.className = 'toast toast-' + type;
    const icons = { success: '✓', error: '✕', info: 'ℹ' };
    toast.innerHTML = '<span>' + (icons[type] || '') + '</span> ' + msg;
    container.appendChild(toast);
    setTimeout(() => { toast.style.opacity = '0'; setTimeout(() => toast.remove(), 300); }, 3500);
}

// ── Page Navigation ──
function showPage(pageId) {
    document.querySelectorAll('.page-section').forEach(p => p.classList.remove('active'));
    document.querySelectorAll('.nav-item').forEach(n => n.classList.remove('active'));
    const page = document.getElementById(pageId);
    if (page) page.classList.add('active');
    const nav = document.querySelector(`[data-page="${pageId}"]`);
    if (nav) nav.classList.add('active');
    document.querySelector('.topbar-left h1').textContent = getPageTitle(pageId);
    // Trigger page-specific loaders
    if (pageId === 'page-dashboard') loadDashboard();
    else if (pageId === 'page-complaints') loadComplaints();
    else if (pageId === 'page-register') { /* form ready */ }
    else if (pageId === 'page-profile') loadProfile();
    else if (pageId === 'page-users') loadUsers();
    else if (pageId === 'page-assigned') loadAssigned();
}

function getPageTitle(id) {
    const titles = {
        'page-dashboard': 'Dashboard',
        'page-register': 'Register Complaint',
        'page-complaints': 'My Complaints',
        'page-assigned': 'Assigned Complaints',
        'page-profile': 'Profile',
        'page-users': 'Manage Users'
    };
    return titles[id] || 'Dashboard';
}

// ── Initialize App ──
function initApp() {
    if (!requireAuth()) return;
    currentUser = getUser();
    if (!currentUser) { logout(); return; }

    // Set user info in sidebar
    const avatar = document.getElementById('userAvatar');
    const userName = document.getElementById('userName');
    const userRole = document.getElementById('userRole');
    if (avatar) avatar.textContent = currentUser.name.charAt(0).toUpperCase();
    if (userName) userName.textContent = currentUser.name;
    if (userRole) userRole.textContent = currentUser.role;

    // Show/hide role-specific nav items
    document.querySelectorAll('[data-role]').forEach(el => {
        const roles = el.dataset.role.split(',');
        el.style.display = roles.includes(currentUser.role) ? '' : 'none';
    });

    // Load notifications count
    loadNotifCount();

    // Show dashboard
    showPage('page-dashboard');
}

// ── Notifications ──
async function loadNotifCount() {
    const data = await api('/api/notifications');
    if (data && data.unreadCount > 0) {
        const badge = document.getElementById('notifBadge');
        if (badge) { badge.textContent = data.unreadCount; badge.style.display = 'flex'; }
    }
}

async function toggleNotifPanel() {
    const panel = document.getElementById('notifPanel');
    panel.classList.toggle('open');
    if (panel.classList.contains('open')) {
        const data = await api('/api/notifications');
        const list = document.getElementById('notifList');
        if (data && data.notifications) {
            if (data.notifications.length === 0) {
                list.innerHTML = '<div style="text-align:center;padding:40px;color:var(--text-muted)">No notifications</div>';
            } else {
                list.innerHTML = data.notifications.map(n => `
                    <div class="notif-item ${n.isRead ? '' : 'unread'}">
                        <div class="notif-msg">${n.message}</div>
                        <div class="notif-time">${n.createdAt}</div>
                    </div>
                `).join('');
            }
        }
    }
}

async function markAllRead() {
    await api('/api/notifications/read-all', { method: 'POST' });
    const badge = document.getElementById('notifBadge');
    if (badge) badge.style.display = 'none';
    document.querySelectorAll('.notif-item.unread').forEach(el => el.classList.remove('unread'));
    showToast('All notifications marked as read', 'success');
}

// ── Profile ──
function loadProfile() {
    const user = getUser();
    if (!user) return;
    const el = document.getElementById('profileContent');
    if (!el) return;
    el.innerHTML = `
        <div class="card profile-card" style="margin:0 auto">
            <div class="profile-avatar">${user.name.charAt(0).toUpperCase()}</div>
            <h2 style="font-size:1.3rem;font-weight:700">${user.name}</h2>
            <span class="badge badge-${user.role === 'admin' ? 'escalated' : user.role === 'officer' ? 'assigned' : 'resolved'}"
                  style="margin-top:8px">${user.role.toUpperCase()}</span>
            <div class="profile-details">
                <div class="detail-row"><span class="detail-label">Email</span><span class="detail-value">${user.email}</span></div>
                <div class="detail-row"><span class="detail-label">Phone</span><span class="detail-value">${user.phone || 'N/A'}</span></div>
                <div class="detail-row"><span class="detail-label">Role</span><span class="detail-value">${user.role}</span></div>
                <div class="detail-row"><span class="detail-label">Department</span><span class="detail-value">${user.department || 'General'}</span></div>
                <div class="detail-row"><span class="detail-label">Member Since</span><span class="detail-value">${user.createdAt || 'N/A'}</span></div>
            </div>
            <button class="btn btn-danger" style="margin-top:24px;width:100%" onclick="logout()">Logout</button>
        </div>
    `;
}

// ── User Management (Admin) ──
async function loadUsers() {
    const el = document.getElementById('usersContent');
    if (!el) return;
    el.innerHTML = '<div class="spinner" style="margin:40px auto;display:block"></div>';
    const data = await api('/api/users');
    if (!data || !data.users) { el.innerHTML = '<p>Failed to load users</p>'; return; }
    let html = `<table class="data-table"><thead><tr>
        <th>ID</th><th>Name</th><th>Email</th><th>Role</th><th>Department</th><th>Actions</th>
    </tr></thead><tbody>`;
    for (const u of data.users) {
        html += `<tr>
            <td>${u.id}</td><td>${u.name}</td><td>${u.email}</td>
            <td><span class="badge badge-${u.role === 'admin' ? 'escalated' : u.role === 'officer' ? 'assigned' : 'resolved'}">${u.role}</span></td>
            <td>${u.department || '-'}</td>
            <td>${u.id !== currentUser.id ? `<button class="btn btn-danger" style="padding:4px 12px;font-size:0.75rem" onclick="deleteUser(${u.id})">Delete</button>` : '-'}</td>
        </tr>`;
    }
    html += '</tbody></table>';
    el.innerHTML = html;
}

async function deleteUser(id) {
    if (!confirm('Delete this user?')) return;
    const data = await api('/api/users/' + id, { method: 'DELETE' });
    if (data && data.success) { showToast('User deleted', 'success'); loadUsers(); }
}

// ── Mobile sidebar toggle ──
function toggleSidebar() {
    document.querySelector('.sidebar').classList.toggle('open');
    document.querySelector('.sidebar-overlay').classList.toggle('show');
}

// Init on load
document.addEventListener('DOMContentLoaded', initApp);
