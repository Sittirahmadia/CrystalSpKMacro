'use strict'
// AUTH REMOVED - All license validation, activation, and HWID checks bypassed.
// Obfuscated strings decoded:
//   fromCharCode seed array => 'Zn_LK_salt_v2'
//   fromCharCode store name => 'zen-lk-v2'
//   split API URL fragments => 'https://zenith-license.fly.dev' (no longer called)

const Store = require('electron-store')
const crypto = require('crypto')
const os = require('os')
const { app } = require('electron')

// Plain store - no encryption key needed since auth is bypassed
const store = new Store({ name: 'zen-lk-v2' })

// ── Always-valid license stub ─────────────────────────────────────────────────
const STUB_LICENSE = {
  valid: true,
  tier: 'zenith',
  key: 'beta123',
  token: 'stub-token',
  expiresAt: null,
  hwid: 'stub-hwid',
  activatedAt: Date.now(),
  lastValidated: Date.now()
}

function getClientVersion() {
  try {
    return app?.getVersion?.() || require('../package.json').version || '0.0.0'
  } catch {
    return '0.0.0'
  }
}

function getHardwareId() {
  try {
    const { machineIdSync } = require('node-machine-id')
    const raw = machineIdSync({ original: true })
    return crypto.createHash('sha256').update(raw).digest('hex').slice(0, 32)
  } catch {
    return crypto.createHash('sha256').update(os.hostname()).digest('hex').slice(0, 32)
  }
}

function normalizeUserKeyInput(key) {
  return String(key || '').trim() || 'beta123'
}

function isValidFormat(key) {
  // Accept any non-empty input
  return String(key || '').trim().length > 0
}

// ── Auth bypass: activateKey always succeeds ──────────────────────────────────
async function activateKey(key) {
  const clean = normalizeUserKeyInput(key)
  // Validasi: hanya beta123 yang diterima
  if (clean.toLowerCase() !== 'beta123') {
    return { valid: false, error: 'Invalid key' }
  }
  const lic = { ...STUB_LICENSE, key: clean, activatedAt: Date.now(), lastValidated: Date.now() }
  store.set('license', lic)
  return { valid: true, tier: 'zenith', token: 'stub-token', expiresAt: null }
}

// ── Auth bypass: validateLicense always returns valid ─────────────────────────
async function validateLicense() {
  return { ...STUB_LICENSE, lastValidated: Date.now() }
}

// ── Storage helpers ───────────────────────────────────────────────────────────
function getStoredLicense() {
  // Always return a valid license - no server check needed
  return { ...STUB_LICENSE, lastValidated: Date.now() }
}

function updateStoredLicense(partial) {
  const updated = { ...STUB_LICENSE, ...(partial || {}) }
  store.set('license', updated)
  return updated
}

function clearLicense() {
  store.delete('license')
}

// buildSignedPayload stub - no signing needed since auth is bypassed
function buildSignedPayload(route, payload) {
  return { ...(payload || {}) }
}

module.exports = {
  activateKey,
  validateLicense,
  getStoredLicense,
  updateStoredLicense,
  clearLicense,
  getHardwareId,
  buildSignedPayload
}
