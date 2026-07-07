import { useAuth } from '../context/AuthContext';
import { getRoleLabel } from '../utils/api';
import {
  LayoutDashboard, FilePlus, FileText, FolderOpen,
  Users, User, LogOut, Shield, AlertTriangle
} from 'lucide-react';

const NAV_ITEMS = [
  { section: 'Main', items: [
    { id: 'dashboard', label: 'Dashboard', icon: LayoutDashboard, roles: null },
    { id: 'register', label: 'Register Complaint', icon: FilePlus, roles: ['citizen'] },
    { id: 'complaints', label: 'My Complaints', icon: FileText, roles: ['citizen'] },
    { id: 'assigned', label: 'Assigned Complaints', icon: FolderOpen, roles: ['officer', 'senior_officer'] },
    { id: 'all', label: 'All Complaints', icon: FileText, roles: ['admin'] },
    { id: 'escalated', label: 'Escalated Cases', icon: AlertTriangle, roles: ['senior_officer', 'admin'] },
  ]},
  { section: 'Admin', items: [
    { id: 'users', label: 'Manage Users', icon: Users, roles: ['admin'] },
  ]},
  { section: 'Account', items: [
    { id: 'profile', label: 'Profile', icon: User, roles: null },
    { id: 'logout', label: 'Logout', icon: LogOut, roles: null, action: true },
  ]},
];

export default function Sidebar({ activePage, onNavigate, isOpen, onClose }) {
  const { user, logout } = useAuth();
  const role = user?.role || 'citizen';

  function handleClick(item) {
    if (item.id === 'logout') {
      logout();
      return;
    }
    onNavigate(item.id);
    onClose?.();
  }

  function isVisible(item) {
    if (!item.roles) return true;
    return item.roles.includes(role);
  }

  return (
    <>
      <aside className={`sidebar ${isOpen ? 'open' : ''}`} id="sidebar">
        <div className="sidebar-brand">
          <div className="brand-icon-wrapper">
            <Shield size={24} className="brand-icon" />
          </div>
          <h2>SPCMS</h2>
          <span>Smart Complaint System</span>
        </div>

        <nav className="sidebar-nav">
          {NAV_ITEMS.map(section => {
            const visibleItems = section.items.filter(isVisible);
            if (visibleItems.length === 0) return null;
            return (
              <div className="nav-section" key={section.section}>
                <div className="nav-section-title">{section.section}</div>
                {visibleItems.map(item => (
                  <button
                    key={item.id}
                    className={`nav-item ${activePage === item.id ? 'active' : ''}`}
                    onClick={() => handleClick(item)}
                  >
                    <span className="nav-icon"><item.icon size={18} /></span>
                    <span className="nav-label">{item.label}</span>
                    {item.id === 'logout' && <span className="nav-badge-logout">↗</span>}
                  </button>
                ))}
              </div>
            );
          })}
        </nav>

        <div className="sidebar-footer">
          <div className="sidebar-user">
            <div className="avatar">{user?.name?.charAt(0).toUpperCase() || 'U'}</div>
            <div className="info">
              <div className="name">{user?.name || 'User'}</div>
              <div className="role">{getRoleLabel(user?.role) || 'Citizen'}</div>
            </div>
          </div>
        </div>
      </aside>
      <div className={`sidebar-overlay ${isOpen ? 'show' : ''}`} onClick={onClose} />
    </>
  );
}
