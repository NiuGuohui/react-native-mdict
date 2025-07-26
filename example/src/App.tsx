import { Text, View, StyleSheet, Button } from 'react-native';
import { Mdict } from 'react-native-mdict';
import { useCallback } from 'react';

export default function App() {
  const onSearch = useCallback(async () => {
    const mdict = new Mdict(
      '/data/data/mdict.example/files/English-Chinese-Dictionary.mdx'
    );
    console.log('Start');
    let last = Date.now();
    await mdict.init();
    console.log(`Initialize:${Date.now() - last}`);
    last = Date.now();
    const res = await mdict.search('sadasd');
    console.log(`Result:${Date.now() - last}`);
    console.log(res);
    last = Date.now();
    const list = await mdict.keyList('ab');
    console.log(`KeyList:${Date.now() - last}`);
    console.log(list);
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
