export default {
  components: [
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/ethernet.node',
        linkLayers: [
          {link: 1, id: '*eth'}
        ],
        samples: [
          {
            pcap: 'samples/tcp-ipv4.pcap',
            assert: 'test/eth.json'
          }
        ],
        descriptors: {
          eth: {
            name: 'Ethernet'
          },
          'eth.src': {
            name: 'Source'
          },
          'eth.dst': {
            name: 'Destination'
          },
          'eth.ethType': {
            name: 'EtherType'
          }
        }
      }
    },
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/ipv4.node',
        samples: [
          { pcap: 'samples/tcp-ipv4.pcap' }
        ],
        descriptors: {
          ipv4: {
            name: 'IPv4'
          },
          'ipv4.version': {
            name: 'Version'
          },
          'ipv4.hLen': {
            name: 'Internet Header Length'
          },
          'ipv4.type': {
            name: 'Type Of Service'
          },
          'ipv4.tLen': {
            name: 'Total Length'
          },
          'ipv4.id': {
            name: 'Identification'
          },
          'ipv4.flags': {
            name: 'Flags'
          },
          'ipv4.flags.reserved': {
            name: 'Reserved'
          },
          'ipv4.flags.dontFrag': {
            name: 'Don\'t Fragment'
          },
          'ipv4.flags.moreFrag': {
            name: 'More Fragments'
          },
          'ipv4.fOffset': {
            name: 'Fragment Offset'
          },
          'ipv4.ttl': {
            name: 'TTL'
          },
          'ipv4.protocol': {
            name: 'Protocol'
          },
          'ipv4.checksum': {
            name: 'Header Checksum'
          },
          'ipv4.src': {
            name: 'Source'
          },
          'ipv4.dst': {
            name: 'Destination'
          }
        }
      }
    },
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/ipv6.node',
        descriptors: {
          ipv6: {
            name: 'IPv6'
          },
          'ipv6.version': {
            name: 'Version'
          },
          'ipv6.tClass': {
            name: 'Traffic Class'
          },
          'ipv6.fLevel': {
            name: 'Flow Level'
          },
          'ipv6.pLen': {
            name: 'Payload Length'
          },
          'ipv6.nHeader': {
            name: 'Next Header'
          },
          'ipv6.hLimit': {
            name: 'Hop Limit'
          },
          'ipv6.src': {
            name: 'Source'
          },
          'ipv6.dst': {
            name: 'Destination'
          },
          'ipv6.protocol': {
            name: 'Protocol'
          }
        }
      }
    },
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/udp.node',
        samples: [
          { pcap: 'samples/ntp.pcap' }
        ],
        descriptors: {
          udp: {
            name: 'UDP'
          },
          'udp.src': {
            name: 'Source'
          },
          'udp.dst': {
            name: 'Destination'
          },
          'udp.length': {
            name: 'Length'
          },
          'udp.checksum': {
            name: 'Checksum'
          }
        }
      }
    },
    {
      type: 'dissector',
      dissector: {
        main: 'build/Release/tcp.node',
        descriptors: {
          tcp: {
            name: 'TCP'
          },
          'tcp.src': {
            name: 'Source'
          },
          'tcp.dst': {
            name: 'Destination'
          },
          'tcp.seq': {
            name: 'Sequence number'
          },
          'tcp.ack': {
            name: 'Acknowledgment number'
          },
          'tcp.dOffset': {
            name: 'Data offset'
          },
          'tcp.flags': {
            name: 'Flags'
          },
          'tcp.flags.ns': {
            name: 'NS'
          },
          'tcp.flags.cwr': {
            name: 'CWR'
          },
          'tcp.flags.ece': {
            name: 'ECE'
          },
          'tcp.flags.urg': {
            name: 'URG'
          },
          'tcp.flags.ack': {
            name: 'ACK'
          },
          'tcp.flags.psh': {
            name: 'PSH'
          },
          'tcp.flags.rst': {
            name: 'RST'
          },
          'tcp.flags.syn': {
            name: 'SYN'
          },
          'tcp.flags.fin': {
            name: 'FIN'
          },
          'tcp.window': {
            name: 'Window size'
          },
          'tcp.checksum': {
            name: 'Checksum'
          },
          'tcp.urgent': {
            name: 'Urgent pointer'
          },
          'tcp.options': {
            name: 'Options'
          },
          'tcp.options.nop': {
            name: 'NOP'
          },
          'tcp.options.ts': {
            name: 'Timestamps'
          },
          'tcp.options.ts.mt': {
            name: 'My timestamp'
          },
          'tcp.options.ts.et': {
            name: 'Echo reply timestamp'
          }
        }
      }
    },
    {
      type: 'stream-dissector',
      stream_dissector: {
        main: 'build/Release/tcp-stream.node',
        descriptors: {
          'tcp.stream': {
            name: 'Stream'
          },
          'tcp.stream.length': {
            name: 'Total Received Length'
          },
          'tcp.stream.payloads': {
            name: 'Reassembled Payloads'
          },
          'tcp.stream.lastSeq': {
            name: 'Last Sequence Number'
          }
        }
      }
    },
    {
      type: 'stream-dissector',
      stream_dissector: {
        main: 'build/Release/http-stream.node'
      }
    },
    {
      type: 'listener',
      listener: {
        main: 'build/Release/stream-listener.node'
      }
    }
  ],
  options: [
    {
      id: 'httpPorts',
      name: 'Restrict HTTP Ports',
      type: 'string',
      regexp: '^\\s*([0-9]+,\\s*)*[0-9]*\\s*$',
      default: [80, 8080],
      toJSON: (str) => str.split(',').map((str) => Number.parseInt(str)),
      toString: (json) => json.join(', ')
    }
  ]
}
