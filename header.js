const b64ToArrayBuffer = (() => {
  const chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';

  // Use a lookup table to find the index.
  const lookup = typeof Uint8Array === 'undefined' ? [] : new Uint8Array(256);
  for (let i = 0; i < chars.length; i++) {
      lookup[chars.charCodeAt(i)] = i;
  }

  const decode = (base64) => {
    let bufferLength = base64.length * 0.75,
        len = base64.length,
        i,
        p = 0,
        encoded1,
        encoded2,
        encoded3,
        encoded4;

    if (base64[base64.length - 1] === '=') {
        bufferLength--;
        if (base64[base64.length - 2] === '=') {
            bufferLength--;
        }
    }

    const arraybuffer = new ArrayBuffer(bufferLength),
        bytes = new Uint8Array(arraybuffer);

    for (i = 0; i < len; i += 4) {
        encoded1 = lookup[base64.charCodeAt(i)];
        encoded2 = lookup[base64.charCodeAt(i + 1)];
        encoded3 = lookup[base64.charCodeAt(i + 2)];
        encoded4 = lookup[base64.charCodeAt(i + 3)];

        bytes[p++] = (encoded1 << 2) | (encoded2 >> 4);
        bytes[p++] = ((encoded2 & 15) << 4) | (encoded3 >> 2);
        bytes[p++] = ((encoded3 & 3) << 6) | (encoded4 & 63);
    }

    return arraybuffer;
  };
  return decode;
})();
// globalThis.importScripts = () => {throw new Error()};
globalThis.window = globalThis;
// globalThis.self = globalThis;
globalThis.location = {
  href: '',
};
const makePromise = () => {
  let a, r;
  const p = new Promise((resolve, reject) => {
      a = resolve;
      r = reject;
  });
  p.resolve = a;
  p.reject = r;
  return p;
};
const loadPromise = makePromise();
globalThis.waitForLoad = () => loadPromise;
var Module = {
  onRuntimeInitialized() {
      loadPromise.resolve();
  },
};