// Status page JavaScript

// Theme management
function getPreferredTheme() {
  const stored = localStorage.getItem('theme');
  if (stored) return stored;
  return window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
}

function setTheme(theme) {
  document.documentElement.setAttribute('data-theme', theme);
  localStorage.setItem('theme', theme);
  updateThemeIcon(theme);
}

function updateThemeIcon(theme) {
  const sunIcon = document.querySelector('.icon-sun');
  const moonIcon = document.querySelector('.icon-moon');
  if (sunIcon && moonIcon) {
    if (theme === 'dark') {
      sunIcon.style.display = 'none';
      moonIcon.style.display = 'block';
    } else {
      sunIcon.style.display = 'block';
      moonIcon.style.display = 'none';
    }
  }
}

function toggleTheme() {
  const current = document.documentElement.getAttribute('data-theme') || getPreferredTheme();
  const next = current === 'dark' ? 'light' : 'dark';
  setTheme(next);
}

function toggleMobileMenu() {
  const nav = document.getElementById('nav');
  nav.classList.toggle('header__nav--open');
}

// Stored values
let configLoadTimestamp = null;

// Initialize
document.addEventListener('DOMContentLoaded', function() {
  setTheme(getPreferredTheme());

  updateAllData();
  updateTime();

  // Update data every 5 seconds
  setInterval(updateAllData, 5000);
  // Update time every second
  setInterval(updateTime, 1000);
});

// Listen for system theme changes
window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', (e) => {
  if (!localStorage.getItem('theme')) {
    setTheme(e.matches ? 'dark' : 'light');
  }
});

async function updateAllData() {
  await Promise.all([
    checkApiHealth(),
    fetchMetrics(),
    checkEndpoints()
  ]);

  document.getElementById('lastUpdate').textContent = new Date().toLocaleTimeString('ru-RU');
}

// Check API health
async function checkApiHealth() {
  try {
    const response = await fetch('/api/v2/health', {
      method: 'GET'
    });

    setApiStatus(response.ok);
  } catch (error) {
    setApiStatus(false);
  }
}

function setApiStatus(isOnline) {
  const apiStatusEl = document.getElementById('apiStatus');
  const mainStatusEl = document.getElementById('mainStatus');
  const serverStatusEl = document.getElementById('serverStatus');

  if (isOnline) {
    apiStatusEl.textContent = 'Онлайн';
    apiStatusEl.classList.add('online');
    apiStatusEl.classList.remove('offline');
    mainStatusEl.classList.remove('error');
    serverStatusEl.textContent = 'Все системы работают нормально';
    serverStatusEl.classList.remove('error');
  } else {
    apiStatusEl.textContent = 'Офлайн';
    apiStatusEl.classList.add('offline');
    apiStatusEl.classList.remove('online');
    mainStatusEl.classList.add('error');
    serverStatusEl.textContent = 'API сервер недоступен';
    serverStatusEl.classList.add('error');
  }
}

// Fetch and parse Prometheus metrics
async function fetchMetrics() {
  try {
    const response = await fetch('/metrics');
    if (!response.ok) return;

    const text = await response.text();

    // Parse go_goroutines
    const goroutinesMatch = text.match(/go_goroutines\s+(\d+)/);
    if (goroutinesMatch) {
      document.getElementById('goroutines').textContent = goroutinesMatch[1];
    }

    // Parse go_memstats_alloc_bytes
    const memoryMatch = text.match(/go_memstats_alloc_bytes\s+([\d.e+]+)/);
    if (memoryMatch) {
      const bytes = parseFloat(memoryMatch[1]);
      const megabytes = (bytes / 1024 / 1024).toFixed(1);
      document.getElementById('memoryUsage').textContent = megabytes;
    }

    // Parse caddy_config_last_reload_success_timestamp_seconds for uptime
    const timestampMatch = text.match(/caddy_config_last_reload_success_timestamp_seconds\s+([\d.e+]+)/);
    if (timestampMatch) {
      configLoadTimestamp = parseFloat(timestampMatch[1]);
    }

    // Parse total HTTP requests (sum all caddy_http_requests_total)
    const requestMatches = text.matchAll(/caddy_http_requests_total\{[^}]*\}\s+([\d.e+]+)/g);
    let totalRequests = 0;
    for (const match of requestMatches) {
      totalRequests += parseFloat(match[1]);
    }
    document.getElementById('httpRequests').textContent = Math.round(totalRequests);

  } catch (error) {
    console.error('Failed to fetch metrics:', error);
  }
}

// Update current time and uptime
function updateTime() {
  const now = new Date();
  document.getElementById('currentTime').textContent = now.toLocaleString('ru-RU');

  // Calculate uptime from stored config load timestamp
  if (configLoadTimestamp) {
    const uptimeSeconds = Math.floor(Date.now() / 1000 - configLoadTimestamp);
    document.getElementById('uptime').textContent = formatUptime(uptimeSeconds);
  }
}

function formatUptime(seconds) {
  if (seconds < 0) return '-';

  const days = Math.floor(seconds / 86400);
  const hours = Math.floor((seconds % 86400) / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);

  if (days > 0) {
    return `${days}д ${hours}ч ${minutes}м`;
  } else if (hours > 0) {
    return `${hours}ч ${minutes}м`;
  } else {
    return `${minutes}м`;
  }
}

// Check endpoint availability
async function checkEndpoints() {
  const endpoints = [
    { id: 'dot-api', url: '/api/v2/health' },
    { id: 'dot-swagger', url: '/api/docs' },
    { id: 'dot-legacy', url: '/legacy/' },
    { id: 'dot-docs', url: '/documentation/' },
    { id: 'dot-management', url: '/management/' }
  ];

  await Promise.all(endpoints.map(endpoint => checkEndpoint(endpoint.id, endpoint.url)));
}

async function checkEndpoint(dotId, url) {
  const dotEl = document.getElementById(dotId);
  if (!dotEl) return;

  try {
    const response = await fetch(url, {
      method: 'HEAD'
    });

    if (response.ok || response.status === 405) {
      // 405 is ok for HEAD requests that don't support HEAD
      dotEl.classList.remove('pending', 'red');
      dotEl.classList.add('green');
    } else {
      dotEl.classList.remove('pending', 'green');
      dotEl.classList.add('red');
    }
  } catch (error) {
    dotEl.classList.remove('pending', 'green');
    dotEl.classList.add('red');
  }
}
