#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>
#include <libusb-1.0/libusb.h>

const float I_NOM = 2.067;	// (A)
const float I_SLEW = 0.211; // (A/us)

enum Action
{
	WRITE_DAC_WHITE,
	WRITE_DAC_BLUE,
	LED_SHORT_ON,
	LED_SHORT_OFF,
	LED_ON,
	LED_OFF,
	TRIGGER_HIGH,
	TRIGGER_LOW
};

struct Command
{
	int action;
	uint32_t time;
	uint16_t value;

    friend std::ostream & operator<<(std::ostream &os, const Command& c)
    {
        return os << c.action << " " << c.time << " " << c.value << " ";
    }
};

uint16_t getShortTime(float iset)
{
	if (iset > I_NOM)
		iset = I_NOM;
	else if (iset < 0)
		iset = 0;
	
	return ceil(iset/I_SLEW);
}

float getVset(float iset)
{
	if (iset > I_NOM)
		return 2.40f;
	else if(iset <= 0)
		return 0.0f;
	else
		return iset/I_NOM * 2.40f;
}

std::stringstream makeCommandString(std::vector<Command>& cmds)
{
	std::stringstream ss;

	for(auto& cmd : cmds)
	{
		ss << cmd;
	}
	return ss;
}

std::vector<Command> makePamSequence(unsigned int n_meas, unsigned int t_meas, float i_meas, unsigned int t_pulse, float i_pulse, int rate)
{
	// Do 'n_meas' flashes of 't_meas' microseconds with a current of 'i_meas'.
	// Then do one pulse for 't_pulse' microseconds with a current of 'i_pulse'.
	// All at a rate of 'rate'.
	std::vector<Command> c;

	uint32_t time = 0;							// (us)
	uint32_t t_T    = 1e6 / rate;				// (us)
	uint32_t t_short = getShortTime(i_meas);	// (us)
	uint32_t t_wait = t_T - t_meas - t_short;	// (us)
	uint32_t t_settle = 7;					// (us)

	// Set brightness
	c.push_back({WRITE_DAC_BLUE, time, static_cast<uint16_t> (getVset(i_meas) * 1000) });
	time += t_settle;

	for(unsigned int i = 0; i < n_meas; i++)
	{
		c.push_back( {LED_SHORT_ON, time, 0} );
		c.push_back( {LED_ON, time, 0} );
		time += t_short;

		c.push_back( {LED_SHORT_OFF, time, 0} );
		time += t_meas;

		c.push_back( {LED_OFF, time, 0} );
		time += t_wait;
	}

	time = time - t_wait + t_T - t_meas - getShortTime(i_pulse) - t_settle;
	// Set brightness
	c.push_back({WRITE_DAC_BLUE, time, static_cast<uint16_t> (getVset(i_pulse) * 1000) });
	time += t_settle;

	c.push_back( {LED_SHORT_ON, time, 0} );
	c.push_back( {LED_ON, time, 0} );
	time += getShortTime(i_pulse);

	c.push_back( {LED_SHORT_OFF, time, 0} );
	time += t_pulse;

	c.push_back( {LED_OFF, time, 0} );

	return c;
}

std::vector<Command> makeDeltaTimes(std::vector<Command>& c)
{
	std::vector<Command> c_new = c;
	for (size_t i = 1; i < c.size(); i++)
	{
		c_new[i].time -= c[i-1].time;
	}
	return c_new;
}

static void print_devs(libusb_device **devs)
{
	libusb_device *dev;
	int i = 0, j = 0;
	uint8_t path[8]; 

	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(dev, &desc);
		if (r < 0) {
			fprintf(stderr, "failed to get device descriptor");
			return;
		}

		printf("%04x:%04x (bus %d, device %d)",
			desc.idVendor, desc.idProduct,
			libusb_get_bus_number(dev), libusb_get_device_address(dev));

		r = libusb_get_port_numbers(dev, path, sizeof(path));
		if (r > 0) {
			printf(" path: %d", path[0]);
			for (j = 1; j < r; j++)
				printf(".%d", path[j]);
		}
		printf("\n");
	}
}

int verbose = 0;

static void print_endpoint_comp(const struct libusb_ss_endpoint_companion_descriptor *ep_comp)
{
	printf("      USB 3.0 Endpoint Companion:\n");
	printf("        bMaxBurst:           %u\n", ep_comp->bMaxBurst);
	printf("        bmAttributes:        %02xh\n", ep_comp->bmAttributes);
	printf("        wBytesPerInterval:   %u\n", ep_comp->wBytesPerInterval);
}

static void print_endpoint(const struct libusb_endpoint_descriptor *endpoint)
{
	int i, ret;

	printf("      Endpoint:\n");
	printf("        bEndpointAddress:    %02xh\n", endpoint->bEndpointAddress);
	printf("        bmAttributes:        %02xh\n", endpoint->bmAttributes);
	printf("        wMaxPacketSize:      %u\n", endpoint->wMaxPacketSize);
	printf("        bInterval:           %u\n", endpoint->bInterval);
	printf("        bRefresh:            %u\n", endpoint->bRefresh);
	printf("        bSynchAddress:       %u\n", endpoint->bSynchAddress);

	for (i = 0; i < endpoint->extra_length;) {
		if (LIBUSB_DT_SS_ENDPOINT_COMPANION == endpoint->extra[i + 1]) {
			struct libusb_ss_endpoint_companion_descriptor *ep_comp;

			ret = libusb_get_ss_endpoint_companion_descriptor(NULL, endpoint, &ep_comp);
			if (LIBUSB_SUCCESS != ret)
				continue;

			print_endpoint_comp(ep_comp);

			libusb_free_ss_endpoint_companion_descriptor(ep_comp);
		}

		i += endpoint->extra[i];
	}
}

static void print_altsetting(const struct libusb_interface_descriptor *interface)
{
	uint8_t i;

	printf("    Interface:\n");
	printf("      bInterfaceNumber:      %u\n", interface->bInterfaceNumber);
	printf("      bAlternateSetting:     %u\n", interface->bAlternateSetting);
	printf("      bNumEndpoints:         %u\n", interface->bNumEndpoints);
	printf("      bInterfaceClass:       %u\n", interface->bInterfaceClass);
	printf("      bInterfaceSubClass:    %u\n", interface->bInterfaceSubClass);
	printf("      bInterfaceProtocol:    %u\n", interface->bInterfaceProtocol);
	printf("      iInterface:            %u\n", interface->iInterface);

	for (i = 0; i < interface->bNumEndpoints; i++)
		print_endpoint(&interface->endpoint[i]);
}

static void print_2_0_ext_cap(struct libusb_usb_2_0_extension_descriptor *usb_2_0_ext_cap)
{
	printf("    USB 2.0 Extension Capabilities:\n");
	printf("      bDevCapabilityType:    %u\n", usb_2_0_ext_cap->bDevCapabilityType);
	printf("      bmAttributes:          %08xh\n", usb_2_0_ext_cap->bmAttributes);
}

static void print_ss_usb_cap(struct libusb_ss_usb_device_capability_descriptor *ss_usb_cap)
{
	printf("    USB 3.0 Capabilities:\n");
	printf("      bDevCapabilityType:    %u\n", ss_usb_cap->bDevCapabilityType);
	printf("      bmAttributes:          %02xh\n", ss_usb_cap->bmAttributes);
	printf("      wSpeedSupported:       %u\n", ss_usb_cap->wSpeedSupported);
	printf("      bFunctionalitySupport: %u\n", ss_usb_cap->bFunctionalitySupport);
	printf("      bU1devExitLat:         %u\n", ss_usb_cap->bU1DevExitLat);
	printf("      bU2devExitLat:         %u\n", ss_usb_cap->bU2DevExitLat);
}

static void print_bos(libusb_device_handle *handle)
{
	struct libusb_bos_descriptor *bos;
	uint8_t i;
	int ret;

	ret = libusb_get_bos_descriptor(handle, &bos);
	if (ret < 0)
		return;

	printf("  Binary Object Store (BOS):\n");
	printf("    wTotalLength:            %u\n", bos->wTotalLength);
	printf("    bNumDeviceCaps:          %u\n", bos->bNumDeviceCaps);

	for (i = 0; i < bos->bNumDeviceCaps; i++) {
		struct libusb_bos_dev_capability_descriptor *dev_cap = bos->dev_capability[i];

		if (dev_cap->bDevCapabilityType == LIBUSB_BT_USB_2_0_EXTENSION) {
			struct libusb_usb_2_0_extension_descriptor *usb_2_0_extension;

			ret = libusb_get_usb_2_0_extension_descriptor(NULL, dev_cap, &usb_2_0_extension);
			if (ret < 0)
				return;

			print_2_0_ext_cap(usb_2_0_extension);
			libusb_free_usb_2_0_extension_descriptor(usb_2_0_extension);
		} else if (dev_cap->bDevCapabilityType == LIBUSB_BT_SS_USB_DEVICE_CAPABILITY) {
			struct libusb_ss_usb_device_capability_descriptor *ss_dev_cap;

			ret = libusb_get_ss_usb_device_capability_descriptor(NULL, dev_cap, &ss_dev_cap);
			if (ret < 0)
				return;

			print_ss_usb_cap(ss_dev_cap);
			libusb_free_ss_usb_device_capability_descriptor(ss_dev_cap);
		}
	}

	libusb_free_bos_descriptor(bos);
}

static void print_interface(const struct libusb_interface *interface)
{
	int i;

	for (i = 0; i < interface->num_altsetting; i++)
		print_altsetting(&interface->altsetting[i]);
}

static void print_configuration(struct libusb_config_descriptor *config)
{
	uint8_t i;

	printf("  Configuration:\n");
	printf("    wTotalLength:            %u\n", config->wTotalLength);
	printf("    bNumInterfaces:          %u\n", config->bNumInterfaces);
	printf("    bConfigurationValue:     %u\n", config->bConfigurationValue);
	printf("    iConfiguration:          %u\n", config->iConfiguration);
	printf("    bmAttributes:            %02xh\n", config->bmAttributes);
	printf("    MaxPower:                %u\n", config->MaxPower);

	for (i = 0; i < config->bNumInterfaces; i++)
		print_interface(&config->interface[i]);
}

static void print_device(libusb_device *dev, libusb_device_handle *handle)
{
	struct libusb_device_descriptor desc;
	unsigned char string[256];
	const char *speed;
	int ret;
	uint8_t i;

	switch (libusb_get_device_speed(dev)) {
	case LIBUSB_SPEED_LOW:		speed = "1.5M"; break;
	case LIBUSB_SPEED_FULL:		speed = "12M"; break;
	case LIBUSB_SPEED_HIGH:		speed = "480M"; break;
	case LIBUSB_SPEED_SUPER:	speed = "5G"; break;
	case LIBUSB_SPEED_SUPER_PLUS:	speed = "10G"; break;
	default:			speed = "Unknown";
	}

	ret = libusb_get_device_descriptor(dev, &desc);
	if (ret < 0) {
		fprintf(stderr, "failed to get device descriptor");
		return;
	}

	printf("Dev (bus %u, device %u): %04X - %04X speed: %s\n",
	       libusb_get_bus_number(dev), libusb_get_device_address(dev),
	       desc.idVendor, desc.idProduct, speed);

	if (!handle)
		std::cout << libusb_open(dev, &handle) << std::endl;

	if (handle) {
		if (desc.iManufacturer) {
			ret = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, string, sizeof(string));
			if (ret > 0)
				printf("  Manufacturer:              %s\n", (char *)string);
		}

		if (desc.iProduct) {
			ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, string, sizeof(string));
			if (ret > 0)
				printf("  Product:                   %s\n", (char *)string);
		}

		if (desc.iSerialNumber && verbose) {
			ret = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, string, sizeof(string));
			if (ret > 0)
				printf("  Serial Number:             %s\n", (char *)string);
		}
	}

	if (verbose) {
		for (i = 0; i < desc.bNumConfigurations; i++) {
			struct libusb_config_descriptor *config;

			ret = libusb_get_config_descriptor(dev, i, &config);
			if (LIBUSB_SUCCESS != ret) {
				printf("  Couldn't retrieve descriptors\n");
				continue;
			}

			print_configuration(config);

			libusb_free_config_descriptor(config);
		}

		if (handle && desc.bcdUSB >= 0x0201)
			print_bos(handle);
	}

	if (handle)
		libusb_close(handle);
}


#define MAX_AVG_CURRENT  2067 // mA
#define MAX_V_SET        2400 // mV
#define CURRENT_SLEW     200   // mA/us
inline uint8_t getShortTime(uint16_t vset) {return vset * MAX_AVG_CURRENT / MAX_V_SET / CURRENT_SLEW;}

int main(void)
{
	//std::vector<Command> c = makePamSequence(4, 20, 1.0f, 300000, 2.0f, 8);
	//std::cout << makeCommandString(c).str() << std::endl << std::endl;

	//auto cd = makeDeltaTimes(c);
	//std::cout << makeCommandString(cd).str() << std::endl;

	// Try out libusb
	int r = libusb_init_context(/*ctx=*/NULL, /*options=*/NULL, /*num_options=*/0);
	if (r < 0)
		return r;

	// Search for devices
	libusb_device **devs;
	ssize_t n = libusb_get_device_list(NULL, &devs);
	if (n < 0)
	{
		libusb_exit(NULL);
		return (int)n;
	}

	libusb_device_handle *handle = NULL;
	for (int i = 0; devs[i]; i++)
		print_device(devs[i], handle);
	libusb_free_device_list(devs, 1);

	libusb_exit(NULL);

<<<<<<< Updated upstream
=======
	auto cd = makeDeltaTimes(c);
	std::cout << makeCommandString(cd).str() << std::endl;

	for (uint16_t i = 0; i < 5000; i+=100)
	{
		std::cout << "Vset = " << i << "mV\t" << "Short Time = " << +getShortTime(i) << "us" << std::endl;
	}
>>>>>>> Stashed changes
    return 0;
}