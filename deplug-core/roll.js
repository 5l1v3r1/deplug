import builtin from 'builtin-modules'
import config from './config'
import log from 'electron-log'
import msx from 'msx-optimized'
import path from 'path'
import { rollup } from 'rollup'

export default async function roll (file, rootDir, extern = []) {
  const deplugExtern = ['deplug', 'electron']
  const globalExtern = Object.keys(config.deplug.dependencies)
  const moduleDir = JSON.stringify(path.join(rootDir, 'node_modules'))
  const bundle = await rollup({
    entry: file,
    external: extern.concat(builtin, deplugExtern, globalExtern),
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
