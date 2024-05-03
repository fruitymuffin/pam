#include <cmath>
#include <iostream>
#include <sstream>
#include <vector>

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


int main(void)
{
	std::vector<Command> c = makePamSequence(4, 20, 1.0f, 300000, 2.0f, 8);
	std::cout << makeCommandString(c).str() << std::endl << std::endl;

	auto cd = makeDeltaTimes(c);
	std::cout << makeCommandString(cd).str() << std::endl;
    return 0;
}