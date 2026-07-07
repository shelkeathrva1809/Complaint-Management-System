// ══════════════════════════════════════════════════════════
//  Client-Side Validation — Per User Requirements
// ══════════════════════════════════════════════════════════

export function validateName(name) {
  name = name.trim();
  if (name.length < 2) {
    return { valid: false, error: 'Name must be at least 2 characters' };
  }
  if (!/^[a-zA-Z\s.]+$/.test(name)) {
    return { valid: false, error: 'Name must contain only letters' };
  }
  return { valid: true, error: '' };
}

export function validatePhone(phone) {
  phone = phone.trim();
  if (phone.length !== 10 || !/^\d+$/.test(phone)) {
    return { valid: false, error: 'Phone must be exactly 10 digits' };
  }
  if (!'6789'.includes(phone[0])) {
    return { valid: false, error: 'Phone must start with 6, 7, 8, or 9' };
  }
  return { valid: true, error: '' };
}

export function validateEmail(email) {
  email = email.trim().toLowerCase();
  // Accept any valid email format (user@domain.tld)
  const match = email.match(/^[a-z0-9._%+-]+@[a-z0-9.-]+\.[a-z]{2,}$/);
  if (!match) {
    return { valid: false, error: 'Please enter a valid email address' };
  }
  return { valid: true, error: '' };
}

export function validateSignupEmail(email) {
  email = email.trim().toLowerCase();
  // Accept any valid email format for signup
  const match = email.match(/^[a-z0-9._%+-]+@[a-z0-9.-]+\.[a-z]{2,}$/);
  if (!match) {
    return { valid: false, error: 'Please enter a valid email address' };
  }
  return { valid: true, error: '' };
}

export function validatePassword(pwd) {
  if (pwd.length < 8) return { valid: false, error: 'Password must be at least 8 characters' };
  if (!/[A-Z]/.test(pwd)) return { valid: false, error: 'Password must contain an uppercase letter' };
  if (!/[0-9]/.test(pwd)) return { valid: false, error: 'Password must contain a number' };
  return { valid: true, error: '' };
}

export function validateDescription(desc) {
  desc = desc.trim();
  if (desc.length < 10 || desc.length > 500) {
    return { valid: false, error: 'Description must be 10-500 characters' };
  }
  return { valid: true, error: '' };
}
