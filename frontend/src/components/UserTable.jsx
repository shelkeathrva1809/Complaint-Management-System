import { useState, useEffect } from 'react';
import { useAuth } from '../context/AuthContext';
import { fetchUsers, deleteUser as apiDeleteUser, getRoleLabel, getRoleBadgeClass } from '../utils/api';

export default function UserTable({ onToast }) {
  const { user: currentUser } = useAuth();
  const [users, setUsers] = useState([]);
  const [loading, setLoading] = useState(true);

  async function loadUsers() {
    setLoading(true);
    try {
      const data = await fetchUsers();
      setUsers(data || []);
    } catch {
      setUsers([]);
    }
    setLoading(false);
  }

  useEffect(() => { loadUsers(); }, []);

  async function handleDeleteUser(id) {
    if (!confirm('Delete this user?')) return;
    try {
      const result = await apiDeleteUser(id);
      if (result.success) {
        onToast?.('User deleted', 'success');
        loadUsers();
      } else {
        onToast?.(result.error || 'Failed to delete user', 'error');
      }
    } catch {
      onToast?.('Failed to delete user', 'error');
    }
  }

  if (loading) return (
    <div className="loading-state">
      <span className="spinner" />
      <p>Loading users...</p>
    </div>
  );

  return (
    <div className="table-scroll">
      <table className="data-table">
        <thead>
          <tr>
            <th>ID</th><th>Name</th><th>Email</th><th>Role</th><th>Department</th><th>Actions</th>
          </tr>
        </thead>
        <tbody>
          {users.length === 0 ? (
            <tr><td colSpan={6} className="empty-state">No users found</td></tr>
          ) : (
            users.map(u => (
              <tr key={u.id}>
                <td><strong>#{u.id}</strong></td>
                <td>{u.name}</td>
                <td>{u.email}</td>
                <td>
                  <span className={`badge badge-${getRoleBadgeClass(u.role)}`}>
                    {getRoleLabel(u.role)}
                  </span>
                </td>
                <td>{u.department || '-'}</td>
                <td>
                  {u.id !== currentUser?.id ? (
                    <button className="btn btn-sm btn-danger" onClick={() => handleDeleteUser(u.id)}>
                      Delete
                    </button>
                  ) : <span className="text-muted">Current</span>}
                </td>
              </tr>
            ))
          )}
        </tbody>
      </table>
    </div>
  );
}
