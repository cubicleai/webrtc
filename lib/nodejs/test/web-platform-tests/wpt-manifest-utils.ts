'use strict';
import fs from 'fs';

const EXPECTED_MANIFEST_VERSION = 6;

export const getPossibleTestFilePaths = manifest => {
  const testharnessTests = manifest.items.testharness;

  const allPaths = [];
  for (const containerPath of Object.keys(testharnessTests)) {
    const testFilePaths = testharnessTests[containerPath].map(value => value[0]);
    for (const testFilePath of testFilePaths) {
      // Globally disable worker tests
      if (testFilePath.endsWith('.worker.html') ||
          testFilePath.endsWith('.serviceworker.html') ||
          testFilePath.endsWith('.sharedworker.html')) {
        continue;
      }

      allPaths.push(exports.stripPrefix(testFilePath, ''));
    }
  }

  return allPaths;
};

export const stripPrefix = (string, prefix) => string.substring(prefix.length);

export const readManifest = filename => {
  const manifestString = fs.readFileSync(filename, { encoding: 'utf-8' });
  const manifest = JSON.parse(manifestString);

  if (manifest.version !== EXPECTED_MANIFEST_VERSION) {
    throw new Error(`WPT manifest format mismatch; expected ${EXPECTED_MANIFEST_VERSION} but got ${manifest.version}`);
  }

  return manifest;
};
