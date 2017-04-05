import builtin from 'builtin-modules'
import config from '../config'
import log from 'electron-log'
import msx from 'msx-optimized'
import objpath from 'object-path'
import path from 'path'
import { rollup } from 'rollup'

export default class Component {
  constructor (rootDir, parc, comp) {
    this.type = comp.type
    this.rootDir = rootDir
    this.parc = parc
    this.comp = comp
  }

  async roll (file) {
    const localExtern =
      Object.keys(objpath.get(this.parc, 'dependencies', {}))
    const globalExtern =
      Object.keys(objpath.get(config.deplug, 'dependencies', {}))
    const deplugExtern = ['deplug', 'electron']
    const moduleDir = JSON.stringify(path.join(this.rootDir, 'node_modules'))
    const bundle = await rollup({
      entry: file,
      external: localExtern.concat(globalExtern, builtin, deplugExtern),
      acorn: { ecmaVersion: 8, },
      plugins: [{
        name: 'jsx',
        transform (code) {
          return { code: msx.transform(code, { precompile: false, }), }
        },
      }, {
        name: 'globalPaths',
        banner: `require('module').globalPaths.push(${moduleDir})`,
      }],
      onwarn: (err) => {
        log.warn(err)
      },
    })
    const result = bundle.generate({ format: 'cjs', })
    // eslint-disable-next-line no-new-func
    return new Function('module', '__dirname', result.code)
  }
}
