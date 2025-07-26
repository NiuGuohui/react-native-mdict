import { NativeMdict } from './NativeMdict';

export interface JSMdict {
  init(): Promise<void>;
  keyList(): Promise<string[]>;
  search(keyword: string): Promise<string>;
}

export class Mdict {
  private mdict: JSMdict;

  constructor(file: string) {
    this.mdict = NativeMdict.load(file) as JSMdict;
  }

  init() {
    return this.mdict.init();
  }

  search(keyword: string): Promise<string> {
    return this.mdict.search(keyword);
  }

  keyList() {
    return this.mdict.keyList();
  }
}
