import deplug from './deplug'
import jquery from 'jquery'
import mithril from 'mithril'
import path from 'path'
import { remote } from 'electron'
import roll from './roll'

const { webContents, } = remote
export default async function (argv, tab) {
  try {
    const { Theme, } = await deplug(argv)
    const rootDir = tab.tab.rootDir

    const less = tab.tab.less || ''
    if (less !== '') {
      const lessFile = path.join(rootDir, less)
      const style = await Theme.current.render(lessFile)
      jquery('head').append(jquery('<style>').text(style.css))
    }

    const root = tab.tab.root || ''
    if (root !== '') {
      const rootFile = path.join(rootDir, root)
      const func = await roll(rootFile, rootDir)
      const module = {}
      func(module, rootDir)
      mithril.mount(document.body, module.exports)
    }

    await new Promise((res) => {
      jquery(res)
    })
  } catch (err) {
    remote.getCurrentWebContents().openDevTools()
    // eslint-disable-next-line no-console
    console.error(err)
  } finally {
    for (const wc of webContents.getAllWebContents()) {
      wc.send('tab-deplug-loaded', tab.id)
    }
  }
}
