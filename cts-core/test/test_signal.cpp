#include <catch.hpp>

#include <cts-core/base/signal.h>

using namespace cts::core;

struct TestReceiver : public SignalReceiver
{
	int counter = 0;
	int isum = 0;
	double dsum = 0.0;

	void incrementCounter() {
		int i = 0;
		++counter;
	}
	void plus(int i, double d) { isum += i; dsum += d; }
	void minus(int i, double d) { isum -= i; dsum -= d / 2.0; }
};


TEST_CASE("signal/free-connection-connect-disconnect", "Check connect/disconnect of free connections to signals.")
{
	Signal<> voidSignal;
	REQUIRE(voidSignal.numConnections() == 0);

	int counter = 0;
	int dcounter = 0;

	auto vc1 = voidSignal.connect([&]() { ++counter; });
	voidSignal.emitSignal();
	REQUIRE(voidSignal.numConnections() == 1);
	REQUIRE(counter == 1);

	SignalConnection* vc2 = voidSignal.connect([&]() { ++counter; }, [&](SignalConnection* c) { REQUIRE(c == vc2); ++dcounter; });
	voidSignal.emitSignal();
	REQUIRE(voidSignal.numConnections() == 2);
	REQUIRE(counter == 3);

	REQUIRE(voidSignal.disconnect(vc1) == true);
	voidSignal.emitSignal();
	REQUIRE(voidSignal.numConnections() == 1);
	REQUIRE(counter == 4);

	REQUIRE(voidSignal.disconnect(vc1) == false);
	voidSignal.emitSignal();
	REQUIRE(voidSignal.numConnections() == 1);
	REQUIRE(counter == 5);
	REQUIRE(dcounter == 0);

	REQUIRE(voidSignal.disconnectAll() == 1);
	voidSignal.emitSignal();
	REQUIRE(voidSignal.numConnections() == 0);
	REQUIRE(counter == 5);
	REQUIRE(dcounter == 1);
}


TEST_CASE("signal/signalreceiver-connection-connect-disconnect", "Check connect/disconnect of SignalReceivers to signals.")
{
	Signal<> s1;

	{
		TestReceiver receiver;
		REQUIRE(s1.numConnections() == 0);
		REQUIRE(receiver.numConnections() == 0);

		auto vc1 = s1.connect(&receiver, &TestReceiver::incrementCounter);
		REQUIRE(vc1 != nullptr);
		s1.emitSignal();
		REQUIRE(s1.numConnections() == 1);
		REQUIRE(receiver.numConnections() == 1);
		REQUIRE(receiver.counter == 1);

		auto vc2 = s1.connect(&receiver, &TestReceiver::incrementCounter);
		REQUIRE(vc2 != nullptr);
		s1.emitSignal();
		REQUIRE(s1.numConnections() == 2);
		REQUIRE(receiver.numConnections() == 2);
		REQUIRE(receiver.counter == 3);

		s1.disconnect(vc1);
		s1.emitSignal();
		REQUIRE(s1.numConnections() == 1);
		REQUIRE(receiver.numConnections() == 1);
		REQUIRE(receiver.counter == 4);

		{
			Signal<int, double> s2;
			auto c1 = s2.connect(&receiver, &TestReceiver::plus);
			REQUIRE(s1.numConnections() == 1);
			REQUIRE(s2.numConnections() == 1);
			REQUIRE(receiver.numConnections() == 2);

			s1.emitSignal();
			REQUIRE(receiver.counter == 5);

			s2.emitSignal(1, 2.0);
			REQUIRE(receiver.isum == 1);
			REQUIRE(receiver.dsum == 2.0);

			s2.connect(&receiver, &TestReceiver::minus);
			s2.emitSignal(1, 2.0);
			REQUIRE(receiver.isum == 1);
			REQUIRE(receiver.dsum == 3.0);

			REQUIRE(s2.disconnect(c1) == true);
			REQUIRE(s2.disconnect(c1) == false);
			s2.emitSignal(1, 2.0);
			REQUIRE(receiver.isum == 0);
			REQUIRE(receiver.dsum == 2.0);
		}

		REQUIRE(s1.numConnections() == 1);
		REQUIRE(receiver.numConnections() == 1);
	}

	s1.emitSignal();
	REQUIRE(s1.numConnections() == 0);
}


TEST_CASE("signal/free-connection-parameters", "Check correct parameter forwarding of free connections to signals.")
{
	Signal<int, double> idSignal;
	REQUIRE(idSignal.numConnections() == 0);

	int isum = 0;
	double dsum = 0.0;

	auto vc1 = idSignal.connect([&](int i, double d) { isum += i; dsum += d; });
	idSignal.emitSignal(0, 0.0);
	REQUIRE(isum == 0);
	REQUIRE(dsum == 0.0);

	idSignal.emitSignal(1, 2.0);
	REQUIRE(isum == 1);
	REQUIRE(dsum == 2.0);


	auto vc2 = idSignal.connect([&](int i, double d) { isum -= i; dsum -= d / 2.0; });
	idSignal.emitSignal(0, 0.0);
	REQUIRE(isum == 1);
	REQUIRE(dsum == 2.0);

	idSignal.emitSignal(1, 2.0);
	REQUIRE(isum == 1);
	REQUIRE(dsum == 3.0);

	idSignal.disconnect(vc1);
	idSignal.emitSignal(1, 2.0);
	REQUIRE(isum == 0);
	REQUIRE(dsum == 2.0);
}


TEST_CASE("signal/free-connection-copying", "Check correct copying of signal connections to free connections.")
{
	Signal<> voidSignal;
	int counter = 0;
	int dcounter = 0;

	auto vc1 = voidSignal.connect([&]() { ++counter; });
	voidSignal.emitSignal();
	REQUIRE(voidSignal.numConnections() == 1);
	REQUIRE(counter == 1);

	SignalConnection* vc2 = voidSignal.connect([&]() { ++counter; }, [&](SignalConnection* c) { ++dcounter; });
	voidSignal.emitSignal();
	REQUIRE(voidSignal.numConnections() == 2);
	REQUIRE(counter == 3);
	
	Signal<> scopy(voidSignal);
	REQUIRE(scopy.numConnections() == 2);
	REQUIRE(counter == 3);

	scopy.emitSignal();
	REQUIRE(counter == 5);
	REQUIRE(dcounter == 0);
	
	{
		Signal<> scopy2(scopy);
		REQUIRE(scopy2.numConnections() == 2);
		REQUIRE(counter == 5);

		scopy2.emitSignal();
		REQUIRE(counter == 7);
		REQUIRE(dcounter == 0);

		Signal<> scopy3;
		REQUIRE(scopy3.numConnections() == 0);

		scopy3 = voidSignal;
		REQUIRE(scopy3.numConnections() == 2);
		REQUIRE(counter == 7);

		scopy3.emitSignal();
		REQUIRE(counter == 9);
		REQUIRE(dcounter == 0);
	}

	REQUIRE(counter == 9);
	REQUIRE(dcounter == 2);

	REQUIRE(voidSignal.disconnectAll() == 2);
	REQUIRE(dcounter == 3);

	scopy.emitSignal();
	REQUIRE(counter == 11);

	REQUIRE(scopy.disconnectAll() == 2);
	REQUIRE(dcounter == 4);
}


TEST_CASE("signal/signalreceiver-copying", "Check correct copying of signal connections to SignalReceivers.")
{
	Signal<> s1;
	TestReceiver r1;
	auto c1 = s1.connect(&r1, &TestReceiver::incrementCounter);
	REQUIRE(s1.numConnections() == 1);
	REQUIRE(r1.numConnections() == 1);
	REQUIRE(r1.counter == 0);

	s1.emitSignal();
	REQUIRE(r1.counter == 1);

	{
		Signal<> copy1(s1);
		REQUIRE(copy1.numConnections() == 1);
		REQUIRE(r1.numConnections() == 2);

		copy1.emitSignal();
		REQUIRE(r1.counter == 2);

		{
			Signal<> copy2(s1);
			REQUIRE(copy2.numConnections() == 1);
			REQUIRE(r1.numConnections() == 3);

			copy2.emitSignal();
			REQUIRE(r1.counter == 3);

			Signal<> copy3;
			REQUIRE(copy3.numConnections() == 0);
			
			copy3 = copy2;
			REQUIRE(copy3.numConnections() == 1);
			REQUIRE(r1.numConnections() == 4);

			copy3.emitSignal();
			REQUIRE(r1.counter == 4);
		}

		REQUIRE(r1.numConnections() == 2);
	}

	s1.emitSignal();
	REQUIRE(r1.numConnections() == 1);
	REQUIRE(r1.counter == 5);

	{
		TestReceiver copy1(r1);
		REQUIRE(r1.numConnections() == 1);
		REQUIRE(s1.numConnections() == 2);
		REQUIRE(copy1.numConnections() == 1);
		REQUIRE(copy1.counter == 5);

		s1.emitSignal();
		REQUIRE(r1.counter == 6);
		REQUIRE(copy1.counter == 6);

		{
			TestReceiver copy2(r1);
			REQUIRE(s1.numConnections() == 3);
			REQUIRE(copy2.numConnections() == 1);
			REQUIRE(copy2.counter == 6);

			s1.emitSignal();
			REQUIRE(r1.counter == 7);
			REQUIRE(copy1.counter == 7);
			REQUIRE(copy2.counter == 7);

			TestReceiver copy3;
			REQUIRE(copy3.numConnections() == 0);
			REQUIRE(copy3.counter == 0);

			copy3 = copy2;
			REQUIRE(s1.numConnections() == 4);
			REQUIRE(copy2.numConnections() == 1);
			REQUIRE(copy3.numConnections() == 1);
			REQUIRE(copy2.counter == 7);
			REQUIRE(copy3.counter == 7);

			s1.emitSignal();
			REQUIRE(r1.counter == 8);
			REQUIRE(copy1.counter == 8);
			REQUIRE(copy2.counter == 8);
			REQUIRE(copy3.counter == 8);
		}

		s1.emitSignal();
		REQUIRE(s1.numConnections() == 2);
		REQUIRE(r1.counter == 9);
		REQUIRE(copy1.counter == 9);
	}

	s1.emitSignal();
	REQUIRE(s1.numConnections() == 1);
	REQUIRE(r1.counter == 10);
}
