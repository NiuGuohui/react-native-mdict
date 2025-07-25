import { type JSMdict, NativeMdict } from './NativeMdict';

// @ts-ignore
globalThis.Mdict = NativeMdict;

const obj = NativeMdict.load(
  '/data/data/mdict.example/files/English-Chinese-Dictionary.mdx'
) as JSMdict;

console.log(obj.search('abandon'));

export function multiply(a: number, b: number): number {
  return a * b;
}
