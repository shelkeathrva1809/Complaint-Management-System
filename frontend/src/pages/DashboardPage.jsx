import { useState, useEffect, useCallback, useRef } from 'react';
import { useAuth } from '../context/AuthContext';
import { useTheme } from '../context/ThemeContext';
import { fetchDashboardStats, fetchChartData } from '../utils/api';
import Sidebar from '../components/Sidebar';
import Topbar from '../components/Topbar';
import StatCard from '../components/StatCard';
import BarChart from '../components/BarChart';
import DoughnutChart from '../components/DoughnutChart';
import LineChart from '../components/LineChart';
import ComplaintTable from '../components/ComplaintTable';
import ComplaintForm from '../components/ComplaintForm';
import ProfileCard from '../components/ProfileCard';
import UserTable from '../components/UserTable';

const PAGE_TITLES = {
  dashboard: 'Dashboard',
  register: 'Register Complaint',
  complaints: 'My Complaints',
  assigned: 'Assigned Complaints',
  all: 'All Complaints',
  escalated: 'Escalated Cases',
  profile: 'Profile',
  users: 'Manage Users',
};

export default function DashboardPage() {
  const { user } = useAuth();
  const { theme } = useTheme();
  const [activePage, setActivePage] = useState('dashboard');
  const [sidebarOpen, setSidebarOpen] = useState(false);
  const [stats, setStats] = useState({ total: 0, pending: 0, resolved: 0, emergency: 0, escalated: 0, inProgress: 0 });
  const [chartData, setChartData] = useState(null);
  const [toasts, setToasts] = useState([]);
  const toastIdRef = useRef(0);

  function showToast(msg, type = 'info') {
    const id = ++toastIdRef.current;
    setToasts(prev => [...prev, { id, msg, type }]);
    setTimeout(() => {
      setToasts(prev => prev.filter(t => t.id !== id));
    }, 3500);
  }

  const loadDashboard = useCallback(async () => {
    try {
      const [statsData, charts] = await Promise.all([
        fetchDashboardStats(),
        fetchChartData(),
      ]);
      setStats(statsData);
      setChartData(charts);
    } catch (err) {
      console.error('Failed to load dashboard:', err);
    }
  }, []);

  useEffect(() => {
    loadDashboard();
    // Refresh stats every 30 seconds
    const timer = setInterval(loadDashboard, 30000);
    return () => clearInterval(timer);
  }, [loadDashboard]);

  function handleNavigate(page) {
    setActivePage(page);
    if (page === 'dashboard') loadDashboard();
  }

  const isCitizen = user?.role === 'citizen';
  const isOfficer = user?.role === 'officer';
  const isSeniorOrAdmin = user?.role === 'senior_officer' || user?.role === 'admin';

  return (
    <div className="app-layout">
      <Sidebar
        activePage={activePage}
        onNavigate={handleNavigate}
        isOpen={sidebarOpen}
        onClose={() => setSidebarOpen(false)}
      />

      <main className="main-content">
        <Topbar
          title={PAGE_TITLES[activePage] || 'Dashboard'}
          onToggleSidebar={() => setSidebarOpen(prev => !prev)}
        />

        <div className="page-content">
          {/* DASHBOARD PAGE */}
          <div className={`page-section ${activePage === 'dashboard' ? 'active' : ''}`}>
            <div className="welcome-banner">
              <div className="welcome-text">
                <h2>Welcome back, {user?.name?.split(' ')[0] || 'User'}! 👋</h2>
                <p>{
                  isCitizen ? "Here's what's happening with your complaints today." :
                  isOfficer ? "Here's an overview of your assigned cases." :
                  "Here's the system-wide overview."
                }</p>
              </div>
            </div>
            <div className="stat-grid">
              <StatCard icon="📊" label="Total Complaints" value={stats.total} type="total" trend="up" />
              <StatCard icon="⏳" label="Pending" value={stats.pending} type="pending" trend="up" />
              <StatCard icon="✅" label="Resolved" value={stats.resolved} type="resolved" trend="up" />
              {!isCitizen && <StatCard icon="🚨" label="Emergency" value={stats.emergency} type="emergency" trend="down" />}
              {!isCitizen && <StatCard icon="⚠️" label="Escalated" value={stats.escalated} type="escalated" trend="up" />}
            </div>
            {!isCitizen && (
              <>
                <div className="chart-grid">
                  <BarChart data={chartData?.categories} theme={theme} />
                  <DoughnutChart data={chartData?.statusDist} theme={theme} />
                </div>
                <LineChart data={chartData?.timeline} theme={theme} />
              </>
            )}
          </div>

          {/* REGISTER COMPLAINT PAGE */}
          <div className={`page-section ${activePage === 'register' ? 'active' : ''}`}>
            <ComplaintForm onToast={showToast} />
          </div>

          {/* MY COMPLAINTS PAGE */}
          <div className={`page-section ${activePage === 'complaints' ? 'active' : ''}`}>
            <div className="card table-card">
              <div className="card-header"><h3>📋 Complaint History</h3></div>
              {activePage === 'complaints' && <ComplaintTable type="my" onToast={showToast} />}
            </div>
          </div>

          {/* ASSIGNED COMPLAINTS PAGE (Officer/Admin) */}
          <div className={`page-section ${activePage === 'assigned' ? 'active' : ''}`}>
            <div className="card table-card">
              <div className="card-header"><h3>📂 Assigned to Me</h3></div>
              {activePage === 'assigned' && <ComplaintTable type="assigned" onToast={showToast} />}
            </div>
          </div>

          {/* ALL COMPLAINTS PAGE (Admin) */}
          <div className={`page-section ${activePage === 'all' ? 'active' : ''}`}>
            <div className="card table-card">
              <div className="card-header"><h3>📋 All Complaints (System-wide)</h3></div>
              {activePage === 'all' && <ComplaintTable type="all" onToast={showToast} />}
            </div>
          </div>

          {/* ESCALATED COMPLAINTS PAGE (Senior Officer/Admin) */}
          <div className={`page-section ${activePage === 'escalated' ? 'active' : ''}`}>
            <div className="card table-card">
              <div className="card-header">
                <h3>⚠️ Escalated Cases — Senior Officer Review</h3>
              </div>
              {activePage === 'escalated' && <ComplaintTable type="escalated" onToast={showToast} />}
            </div>
          </div>

          {/* PROFILE PAGE */}
          <div className={`page-section ${activePage === 'profile' ? 'active' : ''}`}>
            <ProfileCard />
          </div>

          {/* MANAGE USERS PAGE */}
          <div className={`page-section ${activePage === 'users' ? 'active' : ''}`}>
            <div className="card table-card">
              <div className="card-header"><h3>👥 All Users</h3></div>
              {activePage === 'users' && <UserTable onToast={showToast} />}
            </div>
          </div>
        </div>
      </main>

      {/* Toast Container */}
      <div className="toast-container">
        {toasts.map(t => (
          <div key={t.id} className={`toast toast-${t.type}`}>
            <span className="toast-icon">
              {t.type === 'success' ? '✓' : t.type === 'error' ? '✕' : 'ℹ'}
            </span>
            <span>{t.msg}</span>
          </div>
        ))}
      </div>
    </div>
  );
}
