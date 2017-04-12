import { ipcRenderer, remote } from 'electron'
import deplug from './deplug'
import jquery from 'jquery'

export default async function (argv) {
  try {
    const { Plugin, GlobalChannel, } = await deplug(argv)
    await Plugin.loadComponents('window')
    await Plugin.loadComponents('tab')
    await new Promise((res) => {
      jquery(res)
    })
    GlobalChannel.emit('core:window:loaded')
    process.nextTick(() => {
      GlobalChannel.emit('core:tab:open', 'Pcap')
    })

  } catch (err) {
    remote.getCurrentWindow().openDevTools()
    // eslint-disable-next-line no-console
    console.error(err)
  } finally {
    ipcRenderer.send('window-deplug-loaded', remote.getCurrentWindow().id)
  }
}
