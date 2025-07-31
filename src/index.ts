import { NativeMdict } from './NativeMdict';

export interface JSMdict {
  init(): Promise<void>;
  lookup(word: string): Promise<string>;
  keyList(keyword: string): Promise<string[]>;
}

export class Mdict {
  private mdict: JSMdict;

  constructor(file: string) {
    this.mdict = NativeMdict.load(file.replace('file://', '')) as JSMdict;
  }

  init() {
    return this.mdict.init();
  }

  lookup(word: string): Promise<string> {
    return this.mdict.lookup(word);
  }

  keyList(keyword: string) {
    return this.mdict.keyList(keyword);
  }
}
