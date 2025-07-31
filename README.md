# react-native-mdict

A mdict(*.mdx/mdd) file parser.(JSI version)

## Installation

```sh
npm install react-native-mdict
```

## Usage


```js
import { Mdict } from 'react-native-mdict';

// ...
// Create mdict instance
const mdict = new Mdict('xxx/Dictionary.mdx');
// Init mdict
await mdict.init();
// lookup and get result
const res = await mdict.lookup('sadasd');
// Search word with specific key
const list = await mdict.keyList('ab');

```

### Tips
Perhaps you have a need to render simple HTML, here is a solution: [react-native-litehtml](https://github.com/NiuGuohui/react-native-litehtml)


## Contributing

See the [contributing guide](CONTRIBUTING.md) to learn how to contribute to the repository and the development workflow.

## License

MIT

---

Made with [create-react-native-library](https://github.com/callstack/react-native-builder-bob)
