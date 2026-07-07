import { useState } from 'react';
import { useNavigate } from 'react-router-dom';
import { useAuth } from '../context/AuthContext';
import { validateName, validatePhone, validateEmail, validateSignupEmail, validatePassword } from '../utils/validation';
import { Eye, EyeOff, LogIn, UserPlus, Shield } from 'lucide-react';

export default function LoginPage() {
  const [tab, setTab] = useState('login');
  const [error, setError] = useState('');
  const [success, setSuccess] = useState('');
  const [loading, setLoading] = useState(false);
  const { login, signup } = useAuth();
  const navigate = useNavigate();

  // Login form state
  const [loginEmail, setLoginEmail] = useState('');
  const [loginPassword, setLoginPassword] = useState('');
  const [showLoginPwd, setShowLoginPwd] = useState(false);

  // Signup form state
  const [signupName, setSignupName] = useState('');
  const [signupPhone, setSignupPhone] = useState('');
  const [signupEmail, setSignupEmail] = useState('');
  const [signupPassword, setSignupPassword] = useState('');
  const [showSignupPwd, setShowSignupPwd] = useState(false);

  // Field-level validation state
  const [fieldErrors, setFieldErrors] = useState({});
  const [fieldValid, setFieldValid] = useState({});

  function updateField(field, value, validator) {
    if (value.trim() === '') {
      setFieldErrors(prev => ({ ...prev, [field]: '' }));
      setFieldValid(prev => ({ ...prev, [field]: false }));
      return;
    }
    const result = validator(value);
    setFieldErrors(prev => ({ ...prev, [field]: result.valid ? '' : result.error }));
    setFieldValid(prev => ({ ...prev, [field]: result.valid }));
  }

  async function handleLogin(e) {
    e.preventDefault();
    setError('');
    setSuccess('');
    const email = loginEmail.trim().toLowerCase();
    const password = loginPassword;
    if (!email || !password) { setError('Please fill all fields'); return; }

    setLoading(true);
    // Simulate network delay
    await new Promise(r => setTimeout(r, 500));
    const result = await login(email, password);
    setLoading(false);

    if (result.success) {
      setSuccess('Login successful! Redirecting...');
      setTimeout(() => navigate('/dashboard'), 800);
    } else {
      setError(result.error);
    }
  }

  async function handleSignup(e) {
    e.preventDefault();
    setError('');
    setSuccess('');

    const name = signupName.trim();
    const phone = signupPhone.trim();
    const email = signupEmail.trim().toLowerCase();
    const password = signupPassword;

    const checks = [
      { ...validateName(name), field: 'Full Name' },
      { ...validatePhone(phone), field: 'Phone' },
      { ...validateSignupEmail(email), field: 'Email' },
      { ...validatePassword(password), field: 'Password' },
    ];
    for (const c of checks) {
      if (!c.valid) { setError(c.error); return; }
    }

    setLoading(true);
    await new Promise(r => setTimeout(r, 500));
    const result = await signup({ name, email, phone, password });
    setLoading(false);

    if (result.success) {
      setSuccess('Account created successfully! Redirecting...');
      setTimeout(() => navigate('/dashboard'), 1000);
    } else {
      setError(result.error);
    }
  }

  function switchTab(t) {
    setTab(t);
    setError('');
    setSuccess('');
    setFieldErrors({});
    setFieldValid({});
  }

  return (
    <div className="login-page">
      {/* Animated background particles */}
      <div className="login-bg-effects">
        <div className="bg-orb bg-orb-1" />
        <div className="bg-orb bg-orb-2" />
        <div className="bg-orb bg-orb-3" />
      </div>

      <div className="login-container">
        <div className="login-card">
          {/* Logo */}
          <div className="login-logo">
            <div className="logo-icon">
              <Shield size={32} />
            </div>
            <h1>SPCMS</h1>
            <p>Smart Public Complaint Management System</p>
          </div>

          {/* Tabs */}
          <div className="auth-tabs">
            <button
              className={`auth-tab ${tab === 'login' ? 'active' : ''}`}
              onClick={() => switchTab('login')}
            >
              <LogIn size={16} /> Login
            </button>
            <button
              className={`auth-tab ${tab === 'signup' ? 'active' : ''}`}
              onClick={() => switchTab('signup')}
            >
              <UserPlus size={16} /> Sign Up
            </button>
            <div className={`tab-indicator ${tab === 'signup' ? 'right' : ''}`} />
          </div>

          {/* Alerts */}
          {success && (
            <div className="alert alert-success">
              <span className="alert-icon">✔</span> {success}
            </div>
          )}
          {error && (
            <div className="alert alert-error">
              <span className="alert-icon">✕</span> {error}
            </div>
          )}

          {/* LOGIN FORM */}
          {tab === 'login' && (
            <form onSubmit={handleLogin} className="auth-form">
              <div className="form-group">
                <label htmlFor="loginEmail">Email Address</label>
                <input
                  type="email"
                  id="loginEmail"
                  placeholder="you@gmail.com"
                  value={loginEmail}
                  onChange={e => setLoginEmail(e.target.value)}
                  autoComplete="email"
                  required
                />
              </div>
              <div className="form-group">
                <label htmlFor="loginPassword">Password</label>
                <div className="password-wrapper">
                  <input
                    type={showLoginPwd ? 'text' : 'password'}
                    id="loginPassword"
                    placeholder="Enter your password"
                    value={loginPassword}
                    onChange={e => setLoginPassword(e.target.value)}
                    autoComplete="current-password"
                    required
                  />
                  <button type="button" className="password-toggle" onClick={() => setShowLoginPwd(!showLoginPwd)} aria-label="Toggle password visibility">
                    {showLoginPwd ? <EyeOff size={18} /> : <Eye size={18} />}
                  </button>
                </div>
              </div>
              <button type="submit" className="btn-primary" disabled={loading}>
                {loading ? <><span className="spinner" /> Logging in...</> : <><LogIn size={18} /> Login</>}
              </button>
              <div className="demo-credentials">
                <p><strong>Demo Accounts:</strong></p>
                <div className="demo-grid">
                  <span>👤 rajesh@gmail.com / Citizen@123 <em>(Citizen)</em></span>
                  <span>👮 sharma@pwd.gov.in / Officer@123 <em>(Officer)</em></span>
                  <span>⭐ senior@spcms.gov.in / Senior@123 <em>(Sr. Officer)</em></span>
                  <span>🛡️ admin@spcms.gov.in / Admin@123 <em>(Admin)</em></span>
                </div>
              </div>
              <div className="auth-switch">
                Don't have an account?{' '}
                <button type="button" onClick={() => switchTab('signup')}>Sign Up</button>
              </div>
            </form>
          )}

          {/* SIGNUP FORM */}
          {tab === 'signup' && (
            <form onSubmit={handleSignup} className="auth-form">
              <div className={`form-group ${fieldErrors.name ? 'error' : fieldValid.name ? 'valid' : ''}`}>
                <label htmlFor="signupName">Full Name</label>
                <input
                  type="text"
                  id="signupName"
                  placeholder="Enter full name (letters only)"
                  value={signupName}
                  onChange={e => { setSignupName(e.target.value); updateField('name', e.target.value, validateName); }}
                  autoComplete="name"
                  required
                />
                {fieldValid.name && <span className="valid-icon">✓</span>}
                {fieldErrors.name && <div className="error-msg">{fieldErrors.name}</div>}
              </div>

              <div className={`form-group ${fieldErrors.email ? 'error' : fieldValid.email ? 'valid' : ''}`}>
                <label htmlFor="signupEmail">Email Address</label>
                <input
                  type="email"
                  id="signupEmail"
                  placeholder="you@gmail.com"
                  value={signupEmail}
                  onChange={e => { setSignupEmail(e.target.value); updateField('email', e.target.value, validateSignupEmail); }}
                  autoComplete="email"
                  required
                />
                {fieldValid.email && <span className="valid-icon">✓</span>}
                {fieldErrors.email && <div className="error-msg">{fieldErrors.email}</div>}
              </div>

              <div className={`form-group ${fieldErrors.phone ? 'error' : fieldValid.phone ? 'valid' : ''}`}>
                <label htmlFor="signupPhone">Phone Number</label>
                <input
                  type="tel"
                  id="signupPhone"
                  placeholder="10-digit number (starts with 6-9)"
                  maxLength={10}
                  value={signupPhone}
                  onChange={e => { setSignupPhone(e.target.value); updateField('phone', e.target.value, validatePhone); }}
                  autoComplete="tel"
                  required
                />
                {fieldValid.phone && <span className="valid-icon">✓</span>}
                {fieldErrors.phone && <div className="error-msg">{fieldErrors.phone}</div>}
              </div>

              <div className={`form-group ${fieldErrors.password ? 'error' : fieldValid.password ? 'valid' : ''}`}>
                <label htmlFor="signupPassword">Password</label>
                <div className="password-wrapper">
                  <input
                    type={showSignupPwd ? 'text' : 'password'}
                    id="signupPassword"
                    placeholder="Min 8 chars, uppercase + number"
                    value={signupPassword}
                    onChange={e => { setSignupPassword(e.target.value); updateField('password', e.target.value, validatePassword); }}
                    autoComplete="new-password"
                    required
                  />
                  <button type="button" className="password-toggle" onClick={() => setShowSignupPwd(!showSignupPwd)} aria-label="Toggle password visibility">
                    {showSignupPwd ? <EyeOff size={18} /> : <Eye size={18} />}
                  </button>
                </div>
                {fieldValid.password && <span className="valid-icon" style={{ top: 38 }}>✓</span>}
                {fieldErrors.password && <div className="error-msg">{fieldErrors.password}</div>}
              </div>

              <button type="submit" className="btn-primary" disabled={loading}>
                {loading ? <><span className="spinner" /> Creating Account...</> : <><UserPlus size={18} /> Create Account</>}
              </button>
              <div className="auth-switch">
                Already have an account?{' '}
                <button type="button" onClick={() => switchTab('login')}>Login</button>
              </div>
            </form>
          )}
        </div>
      </div>
    </div>
  );
}
