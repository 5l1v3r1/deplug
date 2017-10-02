class StreamReader {
  constructor() {
    this._fields = {
      length: 0,
      slices: []
    }
  }

  get length() {
    return this._fields.length
  }

  addPayload(payload) {
    if (typeof payload !== 'object' || payload === null ||
        payload.constructor.name !== 'Payload') {
      throw new TypeError('First argument must be an Uint8Array')
    }
    for (const slice of payload.slices) {
      this.addSlice(slice)
    }
  }

  addSlice(slice) {
    if (!(slice instanceof Uint8Array)) {
      throw new TypeError('First argument must be an Uint8Array')
    }
    this._fields.length += slice.length
    this._fields.slices.push(slice)
  }
}

exports.StreamReader = StreamReader
