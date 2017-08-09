import m from 'mithril'

export default class IPv4Summary {
  view(vnode) {
    const layer = vnode.attrs.layer
    return <span> TTL: { layer.propertyFromId('ttl').value } </span>
  }
}
