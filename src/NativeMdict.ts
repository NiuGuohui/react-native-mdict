import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';
import type { UnsafeObject } from 'react-native/Libraries/Types/CodegenTypes';

export interface Spec extends TurboModule {
  load(file: string): UnsafeObject;
}

export const NativeMdict = TurboModuleRegistry.getEnforcing<Spec>('Mdict');
