import { useEffect, useRef } from 'react';
import { TrendingUp, TrendingDown, Minus } from 'lucide-react';

export default function StatCard({ icon, label, value, type, trend }) {
  const valueRef = useRef(null);
  const prevValue = useRef(0);

  useEffect(() => {
    const el = valueRef.current;
    if (!el) return;
    const start = prevValue.current;
    const target = value;
    const duration = 800;
    const startTime = performance.now();

    function update(now) {
      const elapsed = now - startTime;
      const progress = Math.min(elapsed / duration, 1);
      // Ease out cubic
      const eased = 1 - Math.pow(1 - progress, 3);
      el.textContent = Math.round(start + (target - start) * eased);
      if (progress < 1) requestAnimationFrame(update);
    }
    requestAnimationFrame(update);
    prevValue.current = target;
  }, [value]);

  return (
    <div className={`stat-card ${type}`}>
      <div className="stat-card-top">
        <div className="stat-icon">{icon}</div>
        <div className="stat-trend">
          {trend === 'up' && <TrendingUp size={14} />}
          {trend === 'down' && <TrendingDown size={14} />}
          {!trend && <Minus size={14} />}
        </div>
      </div>
      <div className="stat-value" ref={valueRef}>0</div>
      <div className="stat-label">{label}</div>
    </div>
  );
}
