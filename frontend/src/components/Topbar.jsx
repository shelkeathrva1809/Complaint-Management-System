import { useTheme } from '../context/ThemeContext';
import { useAuth } from '../context/AuthContext';
import { getRoleLabel } from '../utils/api';
import { Menu, Sun, Moon, User } from 'lucide-react';

export default function Topbar({ title, onToggleSidebar }) {
  const { theme, toggleTheme } = useTheme();
  const { user } = useAuth();
  const isDark = theme === 'dark';

  return (
    <header className="topbar">
      <div className="topbar-left">
        <button className="menu-toggle" onClick={onToggleSidebar} aria-label="Toggle sidebar">
          <Menu size={22} />
        </button>
        <h1>{title}</h1>
      </div>
      <div className="topbar-right">
        {/* Dark mode toggle */}
        <button
          className={`theme-toggle-btn ${isDark ? 'dark' : 'light'}`}
          onClick={toggleTheme}
          title={isDark ? 'Switch to Light Mode' : 'Switch to Dark Mode'}
          aria-label="Toggle theme"
        >
          <span className="theme-toggle-track">
            <span className="theme-toggle-thumb">
              {isDark ? <Moon size={12} /> : <Sun size={12} />}
            </span>
          </span>
          <span className="theme-toggle-label">{isDark ? 'Dark' : 'Light'}</span>
        </button>

        {/* User profile */}
        <div className="topbar-user">
          <div className="topbar-avatar">
            {user?.name?.charAt(0).toUpperCase() || <User size={16} />}
          </div>
          <div className="topbar-user-info">
            <span className="topbar-user-name">{user?.name || 'User'}</span>
            <span className="topbar-user-role">{getRoleLabel(user?.role) || 'Citizen'}</span>
          </div>
        </div>
      </div>
    </header>
  );
}
