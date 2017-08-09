import m from 'mithril'
import moment from 'moment'
import { Token } from 'plugkit'
import { Channel, Profile, Session, Renderer } from 'deplug'
import Buffer from 'buffer'

class BooleanValueItem {
  view(vnode) {
    const faClass = vnode.attrs.value ? 'fa-check-square-o' : 'fa-square-o'
    return <span><i class={`fa ${faClass}`}></i></span>
  }
}

class DateValueItem {
  view(vnode) {
    const tsformat = Profile.current.get('layer-list-panel', 'tsformat')
    const ts = moment(vnode.attrs.value)
    const tsString = ts.format(tsformat)
    return <span>{ tsString }</span>
  }
}

class BufferValueItem {
  view(vnode) {
    const maxLen = 6
    const buffer = vnode.attrs.value
    const hex = buffer.slice(0, maxLen).toString('hex') +
      (buffer.length > maxLen ? '...' : '')
    return <span>[{buffer.length} bytes] 0x{ hex }</span>
  }
}

class ArrayValueItem {
  view(vnode) {
    return <ul>{vnode.attrs.value.map((value) => {
      return <li>{m(PropertyValueItem, {
        prop: {value}
      })}</li>
    })}</ul>
  }
}

class ObjectValueItem {
  view(vnode) {
    const obj = vnode.attrs.value
    return <ul>{Object.keys(obj).map((key) => {
      return <li><label>{ key }</label>{m(PropertyValueItem, {
        prop: {value: obj[key]}
      })}</li>
    })}</ul>
  }
}

class PropertyValueItem {
  view(vnode) {
    const prop = vnode.attrs.prop
    if (typeof prop.value === 'boolean') {
      return m(BooleanValueItem, {value: prop.value})
    } else if (prop.value instanceof Date) {
      return m(DateValueItem, {value: prop.value})
    } else if (prop.value instanceof Uint8Array) {
      return m(BufferValueItem, {value: prop.value})
    } else if (Array.isArray(prop.value)) {
      return m(ArrayValueItem, {value: prop.value})
    } else if (typeof prop.value === 'object' &&
        prop.value != null &&
        Reflect.getPrototypeOf(prop.value) === Object.prototype) {
      return m(ObjectValueItem, {value: prop.value})
    }
    const value = (prop.value == null ? '' : prop.value.toString())
    return <span> { value } </span>
  }
}

Renderer.registerProperty('', PropertyValueItem)

function selectRange(range = []) {
  Channel.emit('core:frame:range-selected', range)
}

class PropertyItem {
  constructor() {
    this.expanded = false
  }

  view(vnode) {
    const prop = vnode.attrs.property
    const value = (prop.value == null ? '' : prop.value.toString())
    let faClass = 'fa fa-circle-o'
    if (prop.properties.length) {
      faClass = this.expanded ? 'fa fa-arrow-circle-down' : 'fa fa-arrow-circle-right'
    }
    const range = [
      prop.range[0] + vnode.attrs.dataOffset,
      prop.range[1] + vnode.attrs.dataOffset
    ]
    const name = (vnode.attrs.path in Session.descriptors) ?
      Session.descriptors[vnode.attrs.path].name : Token.string(prop.id)
    const propRenderer = Renderer.forProperty(Token.string(prop.type))
    return <li
        data-range={ `${range[0]}:${range[1]}` }
        onmouseover={ () => selectRange(range) }
        onmouseout= { () => selectRange() }
      >
      <label
        onclick={ () => this.expanded = !this.expanded }
      ><i class={faClass}></i> { name }: </label>
      { m(propRenderer, {prop, path: vnode.attrs.path}) }
      <label
      class="error"
      style={{ display: prop.error ? 'inline' : 'none' }}
      ><i class="fa fa-exclamation-triangle"></i> { prop.error }</label>
      <ul style={{ display: this.expanded ? 'block' : 'none' }}>
        {
          prop.properties.map((prop) => {
            return m(PropertyItem, {
              property: prop,
              path: vnode.attrs.path + '.' + Token.string(prop.id)
            })
          })
        }
      </ul>
    </li>
  }
}

class LayerDefaultItem {
  view (vnode) {
    return <span> {vnode.attrs.layer.id} </span>
  }
}

Renderer.registerLayer('', LayerDefaultItem)

class LayerItem {
  constructor() {
    this.expanded = false
  }

  view(vnode) {
    const layer = vnode.attrs.layer
    let faClass = 'fa fa-circle-o'
    if (layer.children.length + layer.properties.length) {
      faClass = this.expanded ? 'fa fa-arrow-circle-down' : 'fa fa-arrow-circle-right'
    }
    let dataOffset = 0
    let dataLength = 0
    if (layer.parent) {
      const parentPayload = layer.parent.payloads[0]
      const parentAddr = parentPayload.data.addr
      let rootPayload = parentPayload
      for (let parent = layer.parent.parent; parent; parent = parent.parent) {
        rootPayload = parent.payloads[0]
      }
      const rootAddr = rootPayload.data.addr
      dataLength = parentPayload.data.length
      dataOffset = parentAddr[1] - rootAddr[1]
    }
    const range = [
      dataOffset,
      dataOffset + dataLength
    ]
    const layerId = Token.string(layer.id)
    const name = (layerId in Session.descriptors) ?
      Session.descriptors[layerId].name : layerId
    const layerRenderer = Renderer.forLayer(layerId)
    return <ul>
      <li
        data-range={ `${range[0]}:${range[1]}` }
        onmouseover={ () => selectRange(range) }
        onmouseout= { () => selectRange() }
      >
        <h4
          data-layer={layer.tags.join(' ')}
          onclick={ () => this.expanded = !this.expanded }
        ><i class={faClass}></i> { name } { m(layerRenderer, {layer}) }
        <span
        style={{ display: layer.streamId > 0 ? 'inline' : 'none' }}
        ><i class="fa fa-exchange"></i> Stream</span>
        <span
        style={{ display: layer.confidence < 1.0 ? 'inline' : 'none' }}
        ><i class="fa fa-question-circle"></i> { layer.confidence * 100 }%</span>
        </h4>
      </li>
      <div style={{ display: this.expanded ? 'block' : 'none' }}>
      {
        layer.properties.map((prop) => {
          return m(PropertyItem, {
            property: prop,
            dataOffset,
            path: Token.string(layer.id) + '.' + Token.string(prop.id)
          })
        })
      }
      </div>
      <li>
        <ul>
          {
            layer.children.map((child) => {
              return m(LayerItem, {layer: child})
            })
          }
        </ul>
      </li>
    </ul>
  }
}

export default class LayerListView {
  constructor() {
    this.frames = []
    Channel.on('core:frame:selected', (frames) => {
      this.frames = frames
      m.redraw()
    })
  }

  view(vnode) {
    if (!this.frames.length) {
      return <div class="layer-list-view">No frames selected</div>
    }
    return this.frames.map((frame) => {
      const tsformat = Profile.current.get('layer-list-panel', 'tsformat')
      const ts = moment(frame.timestamp)
      const tsString = ts.format(tsformat)
      let length = `${frame.rootLayer.payloads[0].data.length}`
      if (frame.length > frame.rootLayer.payloads[0].data.length) {
        length += ` (actual: ${frame.length})`
      }
      let layers = [ frame.rootLayer ]
      const rootId = Token.string(frame.rootLayer.id)
      if (rootId.startsWith('[')) {
        layers = frame.rootLayer.children
      }
      return <div class="layer-list-view">
        <ul>
          <li>
            <i class="fa fa-circle-o"></i>
            <label> Timestamp: </label>
            <span> { tsString } </span>
          </li>
          <li>
            <i class="fa fa-circle-o"></i>
            <label> Length: </label>
            <span> { length } </span>
          </li>
        </ul>
        {
          layers.map((layer) => {
            return m(LayerItem, {layer})
          })
        }
      </div>
    })
  }
}
