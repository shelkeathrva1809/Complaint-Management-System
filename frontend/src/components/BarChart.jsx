import { Chart as ChartJS, CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend } from 'chart.js';
import { Bar } from 'react-chartjs-2';

ChartJS.register(CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend);

const CATEGORY_COLORS = {
  'Road': '#f59e0b',
  'Water': '#3b82f6',
  'Electricity': '#a855f7',
  'Sanitation': '#10b981',
  'Other': '#64748b',
};

export default function BarChart({ data, theme }) {
  const isDark = theme === 'dark';
  const textColor = isDark ? '#94a3b8' : '#64748b';
  const gridColor = isDark ? 'rgba(148,163,184,0.08)' : 'rgba(0,0,0,0.06)';

  const labels = data?.labels || ['Road', 'Water', 'Electricity'];
  const values = data?.values || [4, 3, 3];

  const chartData = {
    labels,
    datasets: [{
      label: 'Complaints',
      data: values,
      backgroundColor: labels.map(l => CATEGORY_COLORS[l] || '#64748b'),
      borderRadius: 8,
      borderSkipped: false,
      barThickness: 44,
      hoverBackgroundColor: labels.map(l => (CATEGORY_COLORS[l] || '#64748b') + 'dd'),
    }],
  };

  const options = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      legend: { display: false },
      tooltip: {
        backgroundColor: isDark ? '#1e293b' : '#ffffff',
        titleColor: isDark ? '#f1f5f9' : '#0f172a',
        bodyColor: isDark ? '#94a3b8' : '#64748b',
        borderColor: isDark ? '#334155' : '#e2e8f0',
        borderWidth: 1,
        cornerRadius: 8,
        padding: 12,
      },
    },
    scales: {
      y: {
        beginAtZero: true,
        ticks: { color: textColor, stepSize: 1, font: { size: 12 } },
        grid: { color: gridColor },
        border: { display: false },
      },
      x: {
        ticks: { color: textColor, font: { size: 12, weight: '500' } },
        grid: { display: false },
        border: { display: false },
      },
    },
    animation: {
      duration: 1000,
      easing: 'easeOutQuart',
    },
  };

  return (
    <div className="card chart-card">
      <div className="card-header">
        <h3>📊 Complaints by Category</h3>
      </div>
      <div className="chart-container">
        <Bar data={chartData} options={options} />
      </div>
    </div>
  );
}
