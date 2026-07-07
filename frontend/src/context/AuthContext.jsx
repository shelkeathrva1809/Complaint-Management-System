import { createContext, useContext, useState, useCallback } from 'react';
import { apiLogin, apiSignup, setSession, clearSession, getToken, getUser } from '../utils/api';

const AuthContext = createContext(null);

export function AuthProvider({ children }) {
  const [user, setUser] = useState(() => getUser());
  const [token, setTokenState] = useState(() => getToken());

  const login = useCallback(async (email, password) => {
    const result = await apiLogin(email, password);
    if (result.success) {
      setUser(result.user);
      setTokenState(result.token);
    }
    return result;
  }, []);

  const signup = useCallback(async (formData) => {
    const result = await apiSignup(formData);
    if (result.success) {
      setUser(result.user);
      setTokenState(result.token);
    }
    return result;
  }, []);

  const logout = useCallback(() => {
    clearSession();
    setUser(null);
    setTokenState(null);
  }, []);

  const isAuthenticated = !!token && !!user;

  return (
    <AuthContext.Provider value={{ user, token, isAuthenticated, login, signup, logout }}>
      {children}
    </AuthContext.Provider>
  );
}

export function useAuth() {
  const ctx = useContext(AuthContext);
  if (!ctx) throw new Error('useAuth must be used inside AuthProvider');
  return ctx;
}
