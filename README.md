# Moberg

A library for connecting to various input/output libraries
with a common interface. The C api is:

```c
struct moberg_t;
struct moberg_digital_in_t;


const struct moberg_t *moberg_init();

struct moberg_digital_in_t *moberg_open_digital_in(
  const struct moberg_t *handle,
  int channel);
  
```

Config files are formatted as

```
comedi {
    config {
        /* Parsed by parse_config in libmoberg_comedi.so */
        device = /dev/comedi0 ;
        modprobe = [ comedi 8255 comedi_fc mite ni_tio ni_tiocmd ni_pcimio ] ;
        config = [ ni_pcimio ] ;
    }
    /* Moberg mapping[indices] = {driver specific}[indices]
      {driver specific} is parsed by parse_map in libmoberg_comedi.so */
    map digital_in[0:7] = subdevice[4][0:7] ;
}
serial2002 {
    config {
        /* Parsed by parse_config in libmoberg_serial2002.so */
        device = /dev/ttyS0 ;
        baud = 115200 ;
    }
    /* Moberg mapping[indices] = {driver specific}[indices]
      {driver specific} is parsed by parse_map in libmoberg_serial2002.so */
    map digital_in[30:37] = digital_in[0:7] ;
}
```