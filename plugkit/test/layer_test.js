const assert = require('assert')
const {Testing, Token} = require('../test')

describe('Layer', () => {
  describe('#addAttr()', () => {
    it('should throw for wrong arguments', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.addAttr(),   TypeError)
      assert.throws(() => layer.addAttr([]), TypeError)
    })
    it('should return Attr', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal('Attr', layer.addAttr('.dst').constructor.name)
    })
  })
  describe('#addLayer()', () => {
    it('should throw for wrong arguments', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.addLayer(),   TypeError)
      assert.throws(() => layer.addLayer([]), TypeError)
    })
    it('should return Layer', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal('Layer', layer.addLayer('ipv4').constructor.name)
    })
  })
  describe('#addSubLayer()', () => {
    it('should throw for wrong arguments', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.addSubLayer(),   TypeError)
      assert.throws(() => layer.addSubLayer([]), TypeError)
    })
    it('should return Layer', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal('Layer', layer.addSubLayer('ipv4').constructor.name)
    })
  })
  describe('#addPayload()', () => {
    it('should return addPayload', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal('Payload', layer.addPayload().constructor.name)
    })
  })
  describe('#addTag()', () => {
    it('should throw for wrong arguments', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.addTag(),   TypeError)
      assert.throws(() => layer.addTag([]), TypeError)
    })
  })
  describe('#attr()', () => {
    it('should throw for wrong arguments', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.throws(() => layer.attr(),   TypeError)
      assert.throws(() => layer.attr([]), TypeError)
    })
    it('should return null for unknown id', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal(null, layer.attr('zzz'))
      assert.equal(null, layer.attr(123))
    })
    it('should return found Attr', () => {
      const layer = Testing.createLayerInstance('eth')
      layer.addAttr('.dst')
      assert.equal('.dst', layer.attr('.dst').id)
    })
  })
  describe('#id', () => {
    it('should return layer id', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal('eth', layer.id)
    })
  })
  describe('#worker', () => {
    it('should return layer worker', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal(0, layer.worker)
      layer.worker = 15
      assert.equal(15, layer.worker)
    })
  })
  describe('#confidence', () => {
    it('should return layer confidence', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal(3, layer.confidence)
      layer.confidence = 1
      assert.equal(1, layer.confidence)
    })
  })
  describe('#tags', () => {
    it('should return layer tags', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.deepEqual([], layer.tags)
      layer.addTag('@test')
      assert.deepEqual('@test', layer.tags[0])
    })
  })
  describe('#attrs', () => {
    it('should return layer attrs', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.deepEqual([], layer.attrs)
      layer.addAttr('.dst')
      assert.equal(1, layer.attrs.length)
      layer.addAttr('.src')
      assert.equal(2, layer.attrs.length)
    })
  })
  describe('#payloads', () => {
    it('should return layer payloads', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.deepEqual([], layer.payloads)
      layer.addPayload()
      assert.equal(1, layer.payloads.length)
      layer.addPayload()
      assert.equal(2, layer.payloads.length)
    })
  })
  describe('#parent', () => {
    it('should return parent layer', () => {
      const layer = Testing.createLayerInstance('eth')
      assert.equal('Layer', layer.parent.constructor.name)
      assert.equal(null, layer.parent.parent)
    })
  })
})
