import throttle from 'lodash.throttle'
import m from 'mithril'
import { Channel, Panel, Profile, Session } from 'deplug'
import { Pcap, SessionFactory } from 'plugkit'

class FrameItem {
  constructor() {

  }

  oninit(vnode) {
    this.frame = vnode.attrs.session.getFrames(vnode.attrs.seq - 1, 1)[0]
  }

  select() {
    Channel.emit('core:frame:selected', this.frame)
  }

  view(vnode) {
    let seq = vnode.attrs.seq
    let itemHeight = vnode.attrs.itemHeight
    return <div
      class="frame-item"
      data-layer={this.frame.primaryLayer.namespace}
      data-layer-confidence={1}
      data-frame-length={this.frame.length}
      data-frame-capture-length={this.frame.rootLayer.payload.length}
      data-layer-error={this.frame.hasError}
      onclick={() => {this.select()}}
      style={{
        height: `${itemHeight}px`,
        top: `${(seq - 1) * itemHeight}px`
      }}
    >
      <div class="frame-column">{seq}</div>
      <div class="frame-column">{this.frame.primaryLayer.name}</div>
      <div class="frame-column">{this.frame.primaryLayer.propertyFromId('src').summary}</div>
      <div class="frame-column">{this.frame.primaryLayer.propertyFromId('dst').summary}</div>
      <div class="frame-column">{this.frame.length}</div>
      <div class="frame-column">{this.frame.primaryLayer.summary}</div>
    </div>
  }
}

export default class FrameListView {
  constructor() {
    this.frame = {frames: 0}
    this.session = null
    this.viewScrollTop = 0
    this.viewHeight = 0
    this.mapHeight = 100
    this.previousScrollTop = 0

    const profile = Profile.current['$deplug-builtin-frame-list-panel']
    this.columns = profile.columns || [
      {name: 'No', value: 'seq'},
      {name: 'Protocol', value: 'primaryLayer.name'},
      {name: 'Length', value: 'length'},
      {name: 'Summary', value: 'primaryLayer.summary'},
    ]
    this.attrs = Object.assign({
      'layer-confidence': 'primaryLayer.confidence',
      'frame-length': 'length',
      'frame-capture-length': 'rootLayer.payload.length',
    }, profile.attrs)

    this.updateMapThrottle = throttle((vnode) => {
      this.updateMap(vnode)
    }, 200)

    Channel.on('core:pcap:session-created', (sess) => {
      this.session = sess
      this.session.on('frame', (stat) => {
        this.frame = stat
        m.redraw()
      })
      m.redraw()
    })

    let factory = new SessionFactory()
    factory.networkInterface = Pcap.devices[0].id
    for (const diss of Session.dissectors) {
      factory.registerDissector(diss)
    }
    factory.create().then((sess) => {
      sess.startPcap()
      Channel.emit('core:pcap:session-created', sess)
    }, (err) => {
      console.log(err)
    })
  }

  oncreate(vnode) {
    this.frameView = vnode.dom.querySelector('.frame-list-view')
    this.style = vnode.dom.querySelector('.scrollbar-style')
    this.onupdate(vnode)
    this.frameView.addEventListener('scroll', (event) => {
      this.viewHeight = event.target.offsetHeight
      this.viewScrollTop = event.target.scrollTop
      m.redraw()
    })
  }

  onupdate(vnode) {
    this.viewHeight = this.frameView.offsetHeight
    this.viewScrollTop = this.frameView.scrollTop
    this.updateMapThrottle(vnode)

    const maxScrollTop = this.frameView.scrollHeight - this.frameView.clientHeight
    if (this.previousScrollTop <= this.frameView.scrollTop) {
      this.frameView.scrollTop = maxScrollTop
      this.previousScrollTop = maxScrollTop
    } else {
      this.previousScrollTop =
        Math.max(this.previousScrollTop, this.frameView.scrollHeight * 0.8)
    }
  }

  updateMap(vnode) {
    if (this.session && this.session.frame.frames > 0) {
      const dummy = vnode.dom.querySelector('.dummy-item')
      const ctx = dummy.getContext('2d')
      for (let i = 0; i < this.mapHeight; ++i) {
        const index = Math.floor(this.session.frame.frames / this.mapHeight * i)
        const frame = this.session.getFrames(index, 1)[0]
        dummy.setAttribute('data-layer', frame.primaryLayer.namespace)
        if (frame.hasError) {
          dummy.setAttribute('data-layer-error', '')
        } else {
          dummy.removeAttribute('data-layer-error')
        }
        ctx.fillStyle = getComputedStyle(dummy, null).getPropertyValue("background-color")
        ctx.fillRect(0, i, 1, 1)
      }
      const data = dummy.toDataURL("image/png")

      this.style.textContent = `
      .frame-list-view::-webkit-scrollbar {
        background-image: url(${data});
      }
      `
    }
  }

  view(vnode) {
    const itemHeight = 40
    const viewHeight = this.frame.frames * itemHeight
    const margin = 5
    const begin = Math.max(0,
      Math.floor(this.viewScrollTop / itemHeight) - margin)
    const end = Math.min(begin +
      Math.ceil(this.viewHeight / itemHeight) + margin * 2, this.frame.frames)

    return <div>
      <style class="scrollbar-style"></style>
      <canvas
        style="opacity: 0; position: absolute;"
        class="dummy-item"
        data-layer="eth ipv4 tcp"
        width="1"
        height={this.mapHeight}
      ></canvas>
      <div class="frame-list-view">
        <div
          style={{height: `${viewHeight}px`}}
        >
          {
            (new Array(end - begin)).fill().map((dev, index) => {
              const id = index + begin + 1
              return m(FrameItem, {
                key: id,
                seq: id,
                itemHeight: itemHeight,
                columns: this.columns,
                attrs: this.attrs,
                session: this.session
              })
            })
          }
        </div>
      </div>
    </div>
  }
}
