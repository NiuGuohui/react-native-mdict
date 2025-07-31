import { Text, View, StyleSheet, Button } from 'react-native';
import { Mdict } from 'react-native-mdict';
import { useCallback, useEffect, useRef } from 'react';

export default function App() {
  const mdict = useRef<Mdict>(null);

  useEffect(() => {
    mdict.current = new Mdict(
      '/data/data/mdict.example/files/English-Chinese-Dictionary.mdx'
    );
    mdict.current.init();
    console.log('Start');
  }, []);

  const onSearch = useCallback(async () => {
    const res = await mdict.current?.lookup('ab');
    console.log(res);
  }, []);

  return (
    <View style={styles.container}>
      <Text>Result: 123</Text>
      <Button title="search" onPress={onSearch} />
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: 'center',
    justifyContent: 'center',
  },
});
