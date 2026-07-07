import { useState } from 'react';
import { createComplaint } from '../utils/api';
import { useAuth } from '../context/AuthContext';
import { validateDescription } from '../utils/validation';
import { Send, MapPin, AlertTriangle } from 'lucide-react';

export default function ComplaintForm({ onToast }) {
  const { user } = useAuth();
  const [type, setType] = useState('Road');
  const [severity, setSeverity] = useState('Normal');
  const [location, setLocation] = useState('');
  const [description, setDescription] = useState('');
  const [loading, setLoading] = useState(false);

  async function handleSubmit(e) {
    e.preventDefault();
    const dv = validateDescription(description);
    if (!dv.valid) { onToast?.(dv.error, 'error'); return; }
    if (!location.trim()) { onToast?.('Please enter a location', 'error'); return; }

    setLoading(true);
    try {
      const result = await createComplaint({
        type,
        description: description.trim(),
        location: location.trim(),
        severity,
      });
      if (result.success) {
        onToast?.(`Complaint registered! Tracking ID: ${result.complaint?.trackingId || 'N/A'}`, 'success');
        setType('Road');
        setSeverity('Normal');
        setLocation('');
        setDescription('');
      } else {
        onToast?.(result.error || 'Failed to register complaint', 'error');
      }
    } catch (err) {
      onToast?.('Network error. Is the backend running?', 'error');
    }
    setLoading(false);
  }

  return (
    <div className="card form-card">
      <div className="card-header">
        <h3>📝 New Complaint</h3>
      </div>
      <form onSubmit={handleSubmit}>
        <div className="form-row">
          <div className="dash-form-group">
            <label htmlFor="compType">
              <AlertTriangle size={14} /> Complaint Type
            </label>
            <select id="compType" value={type} onChange={e => setType(e.target.value)} required>
              <option value="Road">🛣️ Road / Infrastructure</option>
              <option value="Water">💧 Water Supply</option>
              <option value="Electricity">⚡ Electricity</option>
            </select>
          </div>
          <div className="dash-form-group">
            <label htmlFor="compSeverity">
              <AlertTriangle size={14} /> Severity Level
            </label>
            <select id="compSeverity" value={severity} onChange={e => setSeverity(e.target.value)} required>
              <option value="Low">🟢 Low</option>
              <option value="Normal">🟡 Normal</option>
              <option value="Emergency">🔴 Emergency</option>
            </select>
          </div>
        </div>
        <div className="dash-form-group">
          <label htmlFor="compLocation">
            <MapPin size={14} /> Location
          </label>
          <input
            type="text" id="compLocation"
            placeholder="Enter complaint location"
            value={location}
            onChange={e => setLocation(e.target.value)}
            required
          />
        </div>
        <div className="dash-form-group">
          <label htmlFor="compDesc">Description</label>
          <textarea
            id="compDesc"
            placeholder="Describe the issue in detail (10-500 characters)..."
            value={description}
            onChange={e => setDescription(e.target.value)}
            required
          />
          <div className="char-count">{description.length}/500</div>
        </div>
        <button type="submit" className="btn btn-accent btn-full" disabled={loading}>
          {loading ? <><span className="spinner" /> Submitting...</> : <><Send size={16} /> Submit Complaint</>}
        </button>
      </form>
    </div>
  );
}
