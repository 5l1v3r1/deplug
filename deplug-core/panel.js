const panelList = {}
export default class Panel {
  static mount (slot, component, less) {
    if (!(slot in panelList)) {
      panelList[slot] = []
    }
    panelList[slot].push({
      component,
      less,
    })
  }

  static get (slot) {
    return panelList[slot] || []
  }
}
