// ══════════════════════════════════════════════════════════
//  Dashboard — Charts (Chart.js) + Stats
// ══════════════════════════════════════════════════════════

let barChart, doughnutChart, lineChart;

async function loadDashboard() {
    // Load stats
    const statsData = await api('/api/dashboard/stats');
    if (statsData && statsData.stats) {
        const s = statsData.stats;
        animateCounter('statTotal', s.total || 0);
        animateCounter('statPending', s.pending || 0);
        animateCounter('statResolved', s.resolved || 0);
        animateCounter('statEmergency', s.emergency || 0);
    }

    // Load charts
    const chartData = await api('/api/dashboard/charts');
    if (chartData && chartData.success) {
        renderBarChart(chartData.categories);
        renderDoughnutChart(chartData.statusDist);
        renderLineChart(chartData.timeline);
    }
}

function animateCounter(id, target) {
    const el = document.getElementById(id);
    if (!el) return;
    const start = parseInt(el.textContent) || 0;
    const duration = 600;
    const startTime = performance.now();
    function update(now) {
        const elapsed = now - startTime;
        const progress = Math.min(elapsed / duration, 1);
        const eased = 1 - Math.pow(1 - progress, 3);
        el.textContent = Math.round(start + (target - start) * eased);
        if (progress < 1) requestAnimationFrame(update);
    }
    requestAnimationFrame(update);
}

function getChartColors() {
    const isDark = getTheme() === 'dark';
    return {
        text: isDark ? '#94a3b8' : '#64748b',
        grid: isDark ? 'rgba(148,163,184,0.08)' : 'rgba(0,0,0,0.06)',
        bg: isDark ? '#1a1f36' : '#ffffff'
    };
}

function renderBarChart(data) {
    if (!data) return;
    const ctx = document.getElementById('barChart');
    if (!ctx) return;
    if (barChart) barChart.destroy();
    const colors = getChartColors();

    barChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: data.labels || ['Road', 'Water', 'Electricity'],
            datasets: [{
                label: 'Complaints',
                data: data.values || [0, 0, 0],
                backgroundColor: ['#f59e0b', '#3b82f6', '#a855f7'],
                borderRadius: 8,
                borderSkipped: false,
                barThickness: 40
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: { display: false }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    ticks: { color: colors.text, stepSize: 1 },
                    grid: { color: colors.grid }
                },
                x: {
                    ticks: { color: colors.text },
                    grid: { display: false }
                }
            }
        }
    });
}

function renderDoughnutChart(data) {
    if (!data) return;
    const ctx = document.getElementById('doughnutChart');
    if (!ctx) return;
    if (doughnutChart) doughnutChart.destroy();

    doughnutChart = new Chart(ctx, {
        type: 'doughnut',
        data: {
            labels: data.labels || [],
            datasets: [{
                data: data.values || [],
                backgroundColor: data.labels ? data.labels.map(l => {
                    const c = { 'Pending': '#f59e0b', 'Assigned': '#3b82f6', 'In Progress': '#eab308',
                                'Resolved': '#10b981', 'Closed': '#64748b', 'Escalated': '#a855f7' };
                    return c[l] || '#475569';
                }) : [],
                borderWidth: 0,
                cutout: '70%'
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: {
                legend: {
                    position: 'right',
                    labels: { color: getChartColors().text, padding: 16, usePointStyle: true, pointStyleWidth: 10 }
                }
            }
        }
    });
}

function renderLineChart(data) {
    if (!data) return;
    const ctx = document.getElementById('lineChart');
    if (!ctx) return;
    if (lineChart) lineChart.destroy();
    const colors = getChartColors();

    lineChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: data.labels || [],
            datasets: [{
                label: 'Complaints',
                data: data.values || [],
                borderColor: '#06b6d4',
                backgroundColor: 'rgba(6,182,212,0.1)',
                fill: true,
                tension: 0.4,
                pointBackgroundColor: '#06b6d4',
                pointRadius: 4,
                pointHoverRadius: 6
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            plugins: { legend: { display: false } },
            scales: {
                y: {
                    beginAtZero: true,
                    ticks: { color: colors.text, stepSize: 1 },
                    grid: { color: colors.grid }
                },
                x: {
                    ticks: { color: colors.text, maxTicksLimit: 10 },
                    grid: { display: false }
                }
            }
        }
    });
}

// Auto-refresh every 30 seconds
setInterval(() => {
    const dashPage = document.getElementById('page-dashboard');
    if (dashPage && dashPage.classList.contains('active')) {
        loadDashboard();
    }
}, 30000);
