// ══════════════════════════════════════════════════════════
//  Theme Manager — Dark/Light Mode Toggle
// ══════════════════════════════════════════════════════════

(function() {
    const KEY = 'spcms_theme';

    function getTheme() {
        return localStorage.getItem(KEY) || 'dark';
    }

    function setTheme(theme) {
        document.documentElement.setAttribute('data-theme', theme);
        localStorage.setItem(KEY, theme);
    }

    function toggleTheme() {
        const current = getTheme();
        setTheme(current === 'dark' ? 'light' : 'dark');
    }

    // Apply saved theme on load
    setTheme(getTheme());

    // Expose globally
    window.toggleTheme = toggleTheme;
    window.getTheme = getTheme;
})();
