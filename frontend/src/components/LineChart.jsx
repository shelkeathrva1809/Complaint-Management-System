import { Chart as ChartJS, CategoryScale, LinearScale, PointElement, LineElement, Filler, Tooltip } from 'chart.js';
import { Line } from 'react-chartjs-2';

ChartJS.register(CategoryScale, LinearScale, PointElement, LineElement, Filler, Tooltip);

export default function LineChart({ data, theme }) {
  const isDark = theme === 'dark';
  const textColor = isDark ? '#94a3b8' : '#64748b';
  const gridColor = isDark ? 'rgba(148,163,184,0.08)' : 'rgba(0,0,0,0.06)';

  const labels = data?.labels || ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun'];
  const values = data?.values || [0, 0, 3, 7, 0, 0];

  const chartData = {
    labels,
    datasets: [{
      label: 'Complaints Filed',
      data: values,
      borderColor: '#06b6d4',
      backgroundColor: (ctx) => {
        const gradient = ctx.chart.ctx.createLinearGradient(0, 0, 0, ctx.chart.height);
        gradient.addColorStop(0, 'rgba(6,182,212,0.25)');
        gradient.addColorStop(1, 'rgba(6,182,212,0)');
        return gradient;
      },
      fill: true,
      tension: 0.4,
      pointBackgroundColor: '#06b6d4',
      pointBorderColor: isDark ? '#1a1f36' : '#ffffff',
      pointBorderWidth: 2,
      pointRadius: 5,
      pointHoverRadius: 8,
      pointHoverBackgroundColor: '#22d3ee',
      borderWidth: 2.5,
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
        displayColors: false,
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
        ticks: { color: textColor, maxTicksLimit: 10, font: { size: 12 } },
        grid: { display: false },
        border: { display: false },
      },
    },
    interaction: {
      intersect: false,
      mode: 'index',
    },
    animation: {
      duration: 1500,
      easing: 'easeOutQuart',
    },
  };

  return (
    <div className="card chart-card" style={{ marginBottom: 24 }}>
      <div className="card-header">
        <h3>📈 Complaints Over Time</h3>
      </div>
      <div className="chart-container">
        <Line data={chartData} options={options} />
      </div>
    </div>
  );
}
