import Theme from './theme'
import objpath from 'object-path'
import path from 'path'

export default class Component {
  static create (rootPath, comp) {
    switch (comp.type) {
      case 'theme':
        return new ThemeComponent(comp.type, rootPath, comp)
      case 'window':
        return new WindowComponent(comp.type, rootPath, comp)
      default:
        throw new Error(`unknown component type: ${comp.type}`)
    }
  }

  constructor (type, rootPath, comp) {
    this.type = type
    this.rootPath = rootPath
    this.comp = comp
  }
}

class ThemeComponent extends Component {
  async load () {
    const id = objpath.get(this.comp, 'theme.id', '')
    if (id === '') {
      throw new Error('theme.id field required')
    }

    const name = objpath.get(this.comp, 'theme.name', '')
    if (name === '') {
      throw new Error('theme.name field required')
    }

    const less = objpath.get(this.comp, 'theme.less', '')
    if (less === '') {
      throw new Error('theme.less field required')
    }

    const lessFile = path.join(path.dirname(this.rootPath), less)
    Theme.register(new Theme(id, name, lessFile))
  }
}

class WindowComponent extends Component {
  async load () {
    const less = objpath.get(this.comp, 'window.less', '')
    if (less !== '') {
      const lessFile = path.join(path.dirname(this.rootPath), less)
      await Theme.current.render(lessFile)
    }
  }
}
