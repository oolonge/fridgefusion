<?php
// Router for Adminer behind Caddy proxy at /admin/
$uri = parse_url($_SERVER['REQUEST_URI'], PHP_URL_PATH);
$query = parse_url($_SERVER['REQUEST_URI'], PHP_URL_QUERY);

// Strip /admin prefix if present
if (strpos($uri, '/admin') === 0) {
    $newPath = substr($uri, 6) ?: '/';
    $_SERVER['REQUEST_URI'] = $newPath . ($query ? '?' . $query : '');
    $_SERVER['SCRIPT_NAME'] = '/index.php';
    $_SERVER['PHP_SELF'] = '/index.php';
}

// Fix cookie path for proxied setup
ini_set('session.cookie_path', '/admin/');

// Serve Adminer
require __DIR__ . '/index.php';
