import { Chart as ChartJS, ArcElement, Tooltip, Legend } from 'chart.js';
import { Doughnut } from 'react-chartjs-2';

ChartJS.register(ArcElement, Tooltip, Legend);

const STATUS_COLORS = {
  'Pending': '#f59e0b',
  'Assigned': '#3b82f6',
  'In Progress': '#eab308',
  'Resolved': '#10b981',
  'Closed': '#64748b',
  'Escalated': '#a855f7',
};

export default function DoughnutChart({ data, theme }) {
  const isDark = theme === 'dark';
  const textColor = isDark ? '#94a3b8' : '#64748b';

  const labels = data?.labels || ['Pending', 'In Progress', 'Resolved', 'Escalated', 'Closed'];
  const values = data?.values || [3, 2, 2, 1, 1];

  const chartData = {
    labels,
    datasets: [{
      data: values,
      backgroundColor: labels.map(l => STATUS_COLORS[l] || '#475569'),
      borderWidth: 0,
      cutout: '72%',
      spacing: 3,
      borderRadius: 4,
      hoverOffset: 8,
    }],
  };

  const options = {
    responsive: true,
    maintainAspectRatio: false,
    plugins: {
      legend: {
        position: 'right',
        labels: {
          color: textColor,
          padding: 14,
          usePointStyle: true,
          pointStyleWidth: 10,
          font: { size: 12, weight: '500' },
        },
      },
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
    animation: {
      animateRotate: true,
      duration: 1200,
    },
  };

  // Center text plugin
  const centerTextPlugin = {
    id: 'centerText',
    beforeDraw(chart) {
      const { ctx, width, height } = chart;
      const total = values.reduce((a, b) => a + b, 0);
      ctx.save();
      ctx.font = `700 ${Math.min(width, height) * 0.09}px Inter, sans-serif`;
      ctx.fillStyle = isDark ? '#f1f5f9' : '#0f172a';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      const centerX = chart.chartArea.left + (chart.chartArea.width / 2);
      const centerY = chart.chartArea.top + (chart.chartArea.height / 2);
      ctx.fillText(total, centerX, centerY - 8);
      ctx.font = `500 ${Math.min(width, height) * 0.045}px Inter, sans-serif`;
      ctx.fillStyle = isDark ? '#64748b' : '#94a3b8';
      ctx.fillText('Total', centerX, centerY + 14);
      ctx.restore();
    },
  };

  return (
    <div className="card chart-card">
      <div className="card-header">
        <h3>🎯 Status Distribution</h3>
      </div>
      <div className="chart-container">
        <Doughnut data={chartData} options={options} plugins={[centerTextPlugin]} />
      </div>
    </div>
  );
}
