// ══════════════════════════════════════════════════════════
//  Client-Side Validation — Mirrors C++ Validator class
// ══════════════════════════════════════════════════════════

function validateName(name) {
    name = name.trim();
    if (name.length < 2 || name.length > 50) {
        return { valid: false, error: 'Invalid name. Please enter only letters' };
    }
    if (!/^[a-zA-Z\s]+$/.test(name)) {
        return { valid: false, error: 'Invalid name. Please enter only letters' };
    }
    return { valid: true, error: '' };
}

function validatePhone(phone) {
    phone = phone.trim();
    if (phone.length !== 10 || !/^\d+$/.test(phone)) {
        return { valid: false, error: 'Invalid phone number. Enter a 10-digit valid number' };
    }
    if (!'6789'.includes(phone[0])) {
        return { valid: false, error: 'Invalid phone number. Enter a 10-digit valid number' };
    }
    return { valid: true, error: '' };
}

function validateEmail(email) {
    email = email.trim().toLowerCase();
    const match = email.match(/^[a-z0-9._%+-]+@(gmail|yahoo|outlook)\.com$/);
    if (!match) {
        return { valid: false, error: 'Invalid email. Use gmail/yahoo/outlook only' };
    }
    return { valid: true, error: '' };
}

function validatePassword(pwd) {
    if (pwd.length < 8) return { valid: false, error: 'Password must be at least 8 characters' };
    if (!/[A-Z]/.test(pwd)) return { valid: false, error: 'Password must contain at least one uppercase letter' };
    if (!/[0-9]/.test(pwd)) return { valid: false, error: 'Password must contain at least one number' };
    return { valid: true, error: '' };
}

function validateDescription(desc) {
    desc = desc.trim();
    if (desc.length < 10 || desc.length > 500) {
        return { valid: false, error: 'Description must be 10-500 characters' };
    }
    return { valid: true, error: '' };
}
