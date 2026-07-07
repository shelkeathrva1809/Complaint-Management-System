import { useAuth } from '../context/AuthContext';
import { getRoleLabel, getRoleBadgeClass } from '../utils/api';
import { Mail, Phone, Shield, Building, Calendar, LogOut } from 'lucide-react';

export default function ProfileCard() {
  const { user, logout } = useAuth();
  if (!user) return null;

  const roleBadge = getRoleBadgeClass(user.role);

  const details = [
    { icon: Mail, label: 'Email', value: user.email },
    { icon: Phone, label: 'Phone', value: user.phone || 'N/A' },
    { icon: Shield, label: 'Role', value: getRoleLabel(user.role) },
    { icon: Building, label: 'Department', value: user.department || 'General' },
    { icon: Calendar, label: 'Member Since', value: user.createdAt || 'N/A' },
  ];

  return (
    <div className="card profile-card">
      <div className="profile-header">
        <div className="profile-avatar">{user.name?.charAt(0).toUpperCase()}</div>
        <h2>{user.name}</h2>
        <span className={`badge badge-${roleBadge}`}>
          {getRoleLabel(user.role).toUpperCase()}
        </span>
      </div>
      <div className="profile-details">
        {details.map(d => (
          <div className="detail-row" key={d.label}>
            <span className="detail-label">
              <d.icon size={14} /> {d.label}
            </span>
            <span className="detail-value">{d.value}</span>
          </div>
        ))}
      </div>
      <button className="btn btn-danger btn-full" onClick={logout}>
        <LogOut size={16} /> Logout
      </button>
    </div>
  );
}
