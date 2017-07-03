/* eslint-disable no-process-exit, no-console */

import DissectorComponent from './components/dissector'
import Plugin from './plugin'
import Session from './session'
import StreamDissectorComponent from './components/stream-dissector'
import path from 'path'

Plugin.listPlugins()
.then((list) => {
  const tasks = []
  for (const plugin of list) {
    for (const comp of plugin.compList) {
      if (comp.type === 'dissector') {
        const dissector =
          new DissectorComponent(plugin.rootDir, plugin.pkg, comp)
        tasks.push(dissector.load())
      } else if (comp.type === 'stream-dissector') {
        const dissector =
          new StreamDissectorComponent(plugin.rootDir, plugin.pkg, comp)
        tasks.push(dissector.load())
      }
    }
  }
  return Promise.all(tasks)
})
.then(() => Session.runSampleBenchmark())
.then((results) => {
  for (const result of results) {
    const fps = (result.frames / (result.duration / 1000000000)).toFixed(3)
    console.log(`${path.basename(result.pcap)}:\t\t${fps} frames/sec`)
  }
  const mean = results.reduce((sum, value) => sum +
    (value.frames / (value.duration / 1000000000)), 0) / results.length
  console.log('------------------------')
  console.log(`${mean.toFixed(3)} frames/sec`)

  process.exit(0)
})
.catch((err) => {
  console.warn(err)

  process.exit(-1)
})
