import { useState, useEffect, useCallback } from 'react';
import {
  fetchComplaints, updateComplaintStatus, resolveComplaint,
  acceptResolution, rejectResolution, rateComplaint,
  assignComplaint, reassignComplaint
} from '../utils/api';
import { useAuth } from '../context/AuthContext';
import { Search, Filter, AlertTriangle, Clock } from 'lucide-react';

// ── 24-Hour Countdown Timer Component ──
function CountdownTimer({ updatedAt, createdAt, status }) {
  const [timeLeft, setTimeLeft] = useState('');
  const [urgency, setUrgency] = useState('safe');

  useEffect(() => {
    if (!['Assigned', 'In Progress'].includes(status)) {
      setTimeLeft(status === 'Escalated' ? '⚠️ ESCALATED' : '—');
      setUrgency(status === 'Escalated' ? 'expired' : 'safe');
      return;
    }

    function calc() {
      const baseTime = updatedAt || createdAt;
      if (!baseTime) { setTimeLeft('—'); return; }
      const start = new Date(baseTime.replace(' ', 'T') + 'Z');  // Backend stores UTC
      const deadline = new Date(start.getTime() + 24 * 60 * 60 * 1000);
      const now = new Date();
      const diff = deadline - now;

      if (diff <= 0) {
        setTimeLeft('⚠️ OVERDUE');
        setUrgency('expired');
        return;
      }

      const hours = Math.floor(diff / (1000 * 60 * 60));
      const mins = Math.floor((diff % (1000 * 60 * 60)) / (1000 * 60));
      const secs = Math.floor((diff % (1000 * 60)) / 1000);
      setTimeLeft(`${String(hours).padStart(2, '0')}:${String(mins).padStart(2, '0')}:${String(secs).padStart(2, '0')}`);

      if (hours < 4) setUrgency('critical');
      else if (hours < 12) setUrgency('warning');
      else setUrgency('safe');
    }

    calc();
    const interval = setInterval(calc, 1000);
    return () => clearInterval(interval);
  }, [updatedAt, createdAt, status]);

  return (
    <span className={`countdown countdown-${urgency}`}>
      <Clock size={12} /> {timeLeft}
    </span>
  );
}

export default function ComplaintTable({ type, onToast }) {
  const { user } = useAuth();
  const [complaints, setComplaints] = useState([]);
  const [loading, setLoading] = useState(true);
  const [search, setSearch] = useState('');
  const [statusFilter, setStatusFilter] = useState('');
  const [typeFilter, setTypeFilter] = useState('');
  const [ratingModal, setRatingModal] = useState(null);
  const [selectedRating, setSelectedRating] = useState(0);
  const [ratingFeedback, setRatingFeedback] = useState('');

  const loadData = useCallback(async () => {
    setLoading(true);
    try {
      const data = await fetchComplaints();
      setComplaints(data || []);
    } catch (err) {
      console.error('Failed to load complaints:', err);
      setComplaints([]);
    }
    setLoading(false);
  }, []);

  useEffect(() => {
    loadData();
    const timer = setInterval(loadData, 30000);
    return () => clearInterval(timer);
  }, [loadData]);

  async function handleAssignToSelf(id) {
    const result = await assignComplaint(id);
    if (result.success) {
      onToast?.(`Complaint #${id} assigned to you!`, 'success');
      loadData();
    } else {
      onToast?.(result.error || 'Assignment failed', 'error');
    }
  }

  async function handleUpdateStatus(id, status) {
    if (!status) return;
    let result;
    if (status === 'Resolved') {
      result = await resolveComplaint(id, 'Issue resolved by ' + user.name);
    } else {
      result = await updateComplaintStatus(id, status);
    }
    if (result.success) {
      onToast?.('Status updated to: ' + status, 'success');
      loadData();
    } else {
      onToast?.(result.error || 'Update failed', 'error');
    }
  }

  async function handleSeniorResolve(id) {
    const result = await resolveComplaint(id, 'Resolved by Senior Officer: ' + user.name);
    if (result.success) {
      onToast?.(`Complaint #${id} resolved by Senior Officer`, 'success');
      loadData();
    } else {
      onToast?.(result.error || 'Resolution failed', 'error');
    }
  }

  async function handleSeniorReassign(id) {
    const result = await reassignComplaint(id);
    if (result.success) {
      onToast?.(result.message || `Complaint #${id} reassigned`, 'success');
      loadData();
    } else {
      onToast?.(result.error || 'Reassignment failed', 'error');
    }
  }

  async function handleAcceptResolution(id) {
    const result = await acceptResolution(id);
    if (result.success) {
      onToast?.('Resolution accepted!', 'success');
      setRatingModal(id);
      loadData();
    } else {
      onToast?.(result.error || 'Failed to accept', 'error');
    }
  }

  async function handleRejectResolution(id) {
    if (!confirm('Reject this resolution? The complaint will go back to In Progress.')) return;
    const result = await rejectResolution(id);
    if (result.success) {
      onToast?.('Resolution rejected. Complaint reopened.', 'info');
      loadData();
    } else {
      onToast?.(result.error || 'Failed to reject', 'error');
    }
  }

  async function handleSubmitRating(id) {
    if (!selectedRating) { onToast?.('Please select a rating', 'error'); return; }
    const result = await rateComplaint(id, selectedRating, ratingFeedback);
    if (result.success) {
      onToast?.('Thank you for your feedback! ⭐', 'success');
      setRatingModal(null);
      setSelectedRating(0);
      setRatingFeedback('');
      loadData();
    } else {
      onToast?.(result.error || 'Rating failed', 'error');
    }
  }

  const filtered = complaints.filter(c => {
    if (type === 'escalated' && c.status !== 'Escalated') return false;
    if (type === 'assigned' && !(c.assignedOfficer === user?.id && c.status !== 'Escalated')) return false;
    // type === 'all' or type === 'my' → no extra filtering (backend handles scoping)
    if (statusFilter && c.status !== statusFilter) return false;
    if (typeFilter && c.type !== typeFilter) return false;
    if (search) {
      const q = search.toLowerCase();
      const text = `${c.id} ${c.trackingId} ${c.type} ${c.description} ${c.status} ${c.officerName || ''} ${c.officerEmail || ''}`.toLowerCase();
      if (!text.includes(q)) return false;
    }
    return true;
  });

  if (loading) {
    return (
      <div className="loading-state">
        <span className="spinner" />
        <p>Loading complaints...</p>
      </div>
    );
  }

  const isOfficer = user?.role === 'officer';
  const isSeniorOfficer = user?.role === 'senior_officer';
  const isAdmin = user?.role === 'admin';
  const isCitizen = user?.role === 'citizen';
  const showTimer = isOfficer || isSeniorOfficer || isAdmin;

  return (
    <>
      <div className="table-controls">
        <div className="search-wrapper">
          <Search size={16} className="search-icon" />
          <input
            className="search-input"
            placeholder="Search complaints..."
            value={search}
            onChange={e => setSearch(e.target.value)}
          />
        </div>
        <div className="filter-group">
          <Filter size={14} />
          <select className="filter-select" value={statusFilter} onChange={e => setStatusFilter(e.target.value)}>
            <option value="">All Status</option>
            <option value="Pending">Pending</option>
            <option value="Assigned">Assigned</option>
            <option value="In Progress">In Progress</option>
            <option value="Resolved">Resolved</option>
            <option value="Escalated">Escalated</option>
            <option value="Closed">Closed</option>
          </select>
          <select className="filter-select" value={typeFilter} onChange={e => setTypeFilter(e.target.value)}>
            <option value="">All Types</option>
            <option value="Road">Road</option>
            <option value="Water">Water</option>
            <option value="Electricity">Electricity</option>
          </select>
        </div>
      </div>

      <div className="table-info">
        <span>Showing {filtered.length} of {complaints.length} complaints</span>
      </div>

      <div className="table-scroll">
        <table className="data-table" id="complaintsTable">
          <thead>
            <tr>
              <th>ID</th>
              <th>Type</th>
              <th>Description</th>
              <th>Severity</th>
              <th>Status</th>
              <th>Officer</th>
              {showTimer && <th>⏱️ Deadline</th>}
              <th>Date</th>
              <th>Actions</th>
            </tr>
          </thead>
          <tbody>
            {filtered.length === 0 ? (
              <tr><td colSpan={showTimer ? 9 : 8} className="empty-state">No complaints found</td></tr>
            ) : (
              filtered.map(c => {
                const sevClass = c.severity === 'Emergency' ? 'emergency' : c.severity === 'Normal' ? 'normal' : 'low';
                const badgeClass = c.status === 'In Progress' ? 'progress' : c.status.toLowerCase().replace(' ', '-');

                return (
                  <tr key={c.id} className={c.status === 'Escalated' ? 'escalated-row' : ''}>
                    <td><strong>#{c.id}</strong></td>
                    <td>
                      <span className="type-badge">
                        {c.type === 'Road' ? '🛣️' : c.type === 'Water' ? '💧' : '⚡'} {c.type}
                      </span>
                    </td>
                    <td className="desc-cell" title={c.description}>{c.description}</td>
                    <td><span className={`badge badge-${sevClass}`}>{c.severity}</span></td>
                    <td><span className={`badge badge-${badgeClass}`}>{c.status}</span></td>
                    <td>
                      {c.officerName && c.officerName !== 'Unassigned' ? (
                        <div className="officer-info">
                          <span className="officer-name">{c.officerName}</span>
                          {c.officerEmail && (
                            <span className="officer-email">{c.officerEmail}</span>
                          )}
                        </div>
                      ) : (
                        <span className="text-muted">Unassigned</span>
                      )}
                    </td>
                    {showTimer && (
                      <td>
                        <CountdownTimer
                          updatedAt={c.updatedAt}
                          createdAt={c.createdAt}
                          status={c.status}
                        />
                      </td>
                    )}
                    <td className="date-cell">{c.createdAt || '-'}</td>
                    <td className="actions-cell">
                      {isOfficer && c.status === 'Pending' && (!c.assignedOfficer || c.assignedOfficer === 0) && (
                        <button className="btn btn-sm btn-accent" onClick={() => handleAssignToSelf(c.id)}>
                          Assign to Me
                        </button>
                      )}
                      {(isOfficer && c.assignedOfficer === user.id && ['Assigned', 'In Progress'].includes(c.status)) && (
                        <select className="action-select" onChange={e => handleUpdateStatus(c.id, e.target.value)} defaultValue="">
                          <option value="">Action...</option>
                          <option value="In Progress">Start Working</option>
                          <option value="Resolved">Mark Resolved</option>
                        </select>
                      )}
                      {isSeniorOfficer && c.status === 'Escalated' && (
                        <div className="action-btns">
                          <button className="btn btn-sm btn-accent" onClick={() => handleSeniorResolve(c.id)}>Resolve</button>
                          <button className="btn btn-sm btn-outline" onClick={() => handleSeniorReassign(c.id)}>Reassign</button>
                        </div>
                      )}
                      {isAdmin && c.status === 'Pending' && (!c.assignedOfficer || c.assignedOfficer === 0) && (
                        <button className="btn btn-sm btn-accent" onClick={() => handleAssignToSelf(c.id)}>Assign</button>
                      )}
                      {isAdmin && c.status === 'Escalated' && (
                        <div className="action-btns">
                          <button className="btn btn-sm btn-accent" onClick={() => handleSeniorResolve(c.id)}>Resolve</button>
                          <button className="btn btn-sm btn-outline" onClick={() => handleSeniorReassign(c.id)}>Reassign</button>
                        </div>
                      )}
                      {isAdmin && ['Assigned', 'In Progress'].includes(c.status) && (
                        <select className="action-select" onChange={e => handleUpdateStatus(c.id, e.target.value)} defaultValue="">
                          <option value="">Action...</option>
                          <option value="In Progress">In Progress</option>
                          <option value="Resolved">Resolved</option>
                          <option value="Escalated">Escalate</option>
                        </select>
                      )}
                      {isCitizen && c.status === 'Resolved' && c.resolutionProof && (
                        <div className="action-btns">
                          <button className="btn btn-sm btn-accent" onClick={() => handleAcceptResolution(c.id)}>Accept</button>
                          <button className="btn btn-sm btn-outline" onClick={() => handleRejectResolution(c.id)}>Reject</button>
                        </div>
                      )}
                      {isCitizen && c.status === 'Closed' && c.rating === 0 && (
                        <button className="btn btn-sm btn-accent" onClick={() => setRatingModal(c.id)}>Rate ⭐</button>
                      )}
                      {isCitizen && c.rating > 0 && <span className="rating-display">{'⭐'.repeat(c.rating)}</span>}
                      {isCitizen && !['Resolved', 'Closed'].includes(c.status) && c.rating === 0 && (
                        <span className="text-muted">—</span>
                      )}
                    </td>
                  </tr>
                );
              })
            )}
          </tbody>
        </table>
      </div>

      {type === 'escalated' && filtered.length > 0 && (
        <div className="escalation-details">
          <h4><AlertTriangle size={14} /> Escalated Complaints — These exceeded the 24-hour resolution deadline</h4>
        </div>
      )}

      {ratingModal && (
        <div className="modal-overlay" onClick={() => setRatingModal(null)}>
          <div className="modal-card" onClick={e => e.stopPropagation()}>
            <h3>⭐ Rate Officer Service</h3>
            <p className="modal-subtitle">How would you rate the resolution quality?</p>
            <div className="stars">
              {[1, 2, 3, 4, 5].map(i => (
                <button key={i} className={`star ${i <= selectedRating ? 'active' : ''}`} onClick={() => setSelectedRating(i)}>★</button>
              ))}
            </div>
            <textarea placeholder="Optional feedback..." value={ratingFeedback} onChange={e => setRatingFeedback(e.target.value)} className="modal-textarea" />
            <div className="modal-actions">
              <button className="btn btn-outline" onClick={() => setRatingModal(null)}>Cancel</button>
              <button className="btn btn-accent" onClick={() => handleSubmitRating(ratingModal)}>Submit Rating</button>
            </div>
          </div>
        </div>
      )}
    </>
  );
}
