// ══════════════════════════════════════════════════════════
//  API Utility — Real C++ Backend Integration
//  All calls go through the Vite proxy → C++ WebServer
// ══════════════════════════════════════════════════════════

const SESSION_KEY = 'spcms_session';

// ── Session Helpers (localStorage for JWT persistence) ──
export function getSession() {
  try {
    const s = localStorage.getItem(SESSION_KEY);
    return s ? JSON.parse(s) : null;
  } catch { return null; }
}

export function getToken() {
  return getSession()?.token || null;
}

export function getUser() {
  return getSession()?.user || null;
}

export function setSession(token, user) {
  localStorage.setItem(SESSION_KEY, JSON.stringify({ token, user }));
}

export function clearSession() {
  localStorage.removeItem(SESSION_KEY);
}

// ══════════════════════════════════════════════════════════
//  Core Fetch Wrapper — auto-attaches JWT Authorization
// ══════════════════════════════════════════════════════════

async function apiFetch(url, options = {}) {
  const token = getToken();
  const headers = {
    'Content-Type': 'application/json',
    ...(options.headers || {}),
  };
  if (token) {
    headers['Authorization'] = `Bearer ${token}`;
  }
  try {
    const res = await fetch(url, { ...options, headers });
    const data = await res.json();
    // If token expired or invalid, clear session
    if (res.status === 401) {
      clearSession();
    }
    return data;
  } catch (err) {
    console.error('API Error:', url, err);
    return { success: false, error: 'Network error. Is the backend running?' };
  }
}

// ══════════════════════════════════════════════════════════
//  AUTH API
// ══════════════════════════════════════════════════════════

export async function apiLogin(email, password) {
  const data = await apiFetch('/api/auth/login', {
    method: 'POST',
    body: JSON.stringify({ email, password }),
  });
  if (data.success) {
    setSession(data.token, data.user);
  }
  return data;
}

export async function apiSignup(formData) {
  const data = await apiFetch('/api/auth/signup', {
    method: 'POST',
    body: JSON.stringify({
      name: formData.name,
      email: formData.email,
      phone: formData.phone,
      password: formData.password,
      role: formData.role || 'citizen',
    }),
  });
  if (data.success) {
    setSession(data.token, data.user);
  }
  return data;
}

export async function fetchCurrentUser() {
  return apiFetch('/api/auth/me');
}

// ══════════════════════════════════════════════════════════
//  COMPLAINTS API
// ══════════════════════════════════════════════════════════

export async function fetchComplaints(filters = {}) {
  const params = new URLSearchParams();
  if (filters.status) params.set('status', filters.status);
  if (filters.type) params.set('type', filters.type);
  const query = params.toString();
  const url = '/api/complaints' + (query ? `?${query}` : '');
  const data = await apiFetch(url);
  return data.success ? data.complaints : [];
}

export async function fetchComplaint(id) {
  const data = await apiFetch(`/api/complaints/${id}`);
  return data.success ? data.complaint : null;
}

export async function createComplaint(complaint) {
  return apiFetch('/api/complaints', {
    method: 'POST',
    body: JSON.stringify(complaint),
  });
}

export async function updateComplaintStatus(id, status) {
  return apiFetch(`/api/complaints/${id}/status`, {
    method: 'PATCH',
    body: JSON.stringify({ status }),
  });
}

export async function resolveComplaint(id, proof) {
  return apiFetch(`/api/complaints/${id}/resolve`, {
    method: 'PATCH',
    body: JSON.stringify({ proof }),
  });
}

export async function acceptResolution(id) {
  return apiFetch(`/api/complaints/${id}/accept`, {
    method: 'PATCH',
    body: JSON.stringify({}),
  });
}

export async function rejectResolution(id) {
  return apiFetch(`/api/complaints/${id}/reject`, {
    method: 'PATCH',
    body: JSON.stringify({}),
  });
}

export async function rateComplaint(id, rating, feedback = '') {
  return apiFetch(`/api/complaints/${id}/rate`, {
    method: 'POST',
    body: JSON.stringify({ rating, feedback }),
  });
}

export async function assignComplaint(id) {
  return apiFetch(`/api/complaints/${id}/assign`, {
    method: 'POST',
    body: JSON.stringify({}),
  });
}

export async function escalateComplaint(id) {
  return apiFetch(`/api/complaints/${id}/escalate`, {
    method: 'PATCH',
    body: JSON.stringify({}),
  });
}

export async function reassignComplaint(id, officerId = 0) {
  return apiFetch(`/api/complaints/${id}/reassign`, {
    method: 'PATCH',
    body: JSON.stringify({ officerId }),
  });
}

// ══════════════════════════════════════════════════════════
//  DASHBOARD / ANALYTICS API
// ══════════════════════════════════════════════════════════

export async function fetchDashboardStats() {
  const data = await apiFetch('/api/dashboard/stats');
  if (data.success) {
    return data.stats;
  }
  return { total: 0, pending: 0, resolved: 0, emergency: 0, escalated: 0, inProgress: 0 };
}

export async function fetchChartData() {
  const data = await apiFetch('/api/dashboard/charts');
  if (data.success) {
    return {
      categories: data.categories,
      statusDist: data.statusDist,
      timeline: data.timeline,
      officerRatings: data.officerRatings,
    };
  }
  return null;
}

// ══════════════════════════════════════════════════════════
//  USER MANAGEMENT API (Admin)
// ══════════════════════════════════════════════════════════

export async function fetchUsers() {
  const data = await apiFetch('/api/users');
  return data.success ? data.users : [];
}

export async function deleteUser(id) {
  return apiFetch(`/api/users/${id}`, { method: 'DELETE' });
}

// ══════════════════════════════════════════════════════════
//  NOTIFICATIONS API
// ══════════════════════════════════════════════════════════

export async function fetchNotifications() {
  const data = await apiFetch('/api/notifications');
  return data.success ? { notifications: data.notifications, unreadCount: data.unreadCount } : { notifications: [], unreadCount: 0 };
}

export async function markAllNotificationsRead() {
  return apiFetch('/api/notifications/read-all', { method: 'POST' });
}

// ══════════════════════════════════════════════════════════
//  ROLE HELPERS (no change — purely frontend logic)
// ══════════════════════════════════════════════════════════

export function getRoleLabel(role) {
  const map = {
    citizen: 'Citizen',
    officer: 'Officer',
    senior_officer: 'Senior Officer',
    admin: 'Admin',
  };
  return map[role] || role;
}

export function getRoleBadgeClass(role) {
  const map = {
    citizen: 'resolved',
    officer: 'assigned',
    senior_officer: 'progress',
    admin: 'escalated',
  };
  return map[role] || 'closed';
}
