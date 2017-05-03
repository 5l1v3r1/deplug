import { Channel } from 'deplug'
import m from 'mithril'

export default class FilterView {
  constructor() {

  }

  oncreate(vnode) {
    Channel.on('core:display-filter:set', (filter) => {
      vnode.dom.value = filter
    })
  }

  press(event) {
    switch (event.code) {
    case 'Enter':
      const filter = event.target.value
      Channel.emit('core:display-filter:set', filter)
    }
    return true
  }

  view(vnode) {
    return <input
      type="text"
      placeholder="Display Filter ..."
      onkeypress={ (event) => {this.press(event)} }
    ></input>
  }
}
