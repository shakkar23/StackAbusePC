#include "usb_cc.hpp"
#include <switch.h>

template <typename C> 
void notify(C args) {
	std::string buf;
	jsoncons::encode_json(args, buf);
	uint32_t size = buf.size();

#ifdef ONEINTERFACE
	usbCommsWrite(&size, sizeof(decltype(size)));
	usbCommsWrite(buf.data(), buf.size());
#else
	usbCommsWriteEx(&size, sizeof(decltype(size)), 0);
	usbCommsWriteEx(buf.data(), buf.size(), 0);
#endif
}



template <typename Cmd, typename R> 
R request(Cmd args) {
	notify<Cmd>(args);
	uint32_t size;

#ifdef ONEINTERFACE
	usbCommsRead(&size, sizeof(decltype(size)));
#else
	usbCommsReadEx(&size, sizeof(decltype(size)), 0);
#endif

	std::vector<char> buf(size);

#ifdef ONEINTERFACE
	usbCommsRead(buf.data(), buf.size());
#else
	usbCommsReadEx(buf.data(), buf.size(), 0);
#endif

	std::string str(buf.begin(), buf.end());
	return jsoncons::decode_json<R>(str);
}



UsbCc::Handle::Handle(CcTypes::Options options, CcTypes::Evaluator evaluator)
	: valid(true) {
		
	auto args = CcRpcTypes::Command(CcRpcTypes::Launch { .options = options, .evaluator = evaluator });
	handle    = request<decltype(args), uint32_t>(args);
}

UsbCc::Handle::~Handle() {
	if(valid) {
		auto args = CcRpcTypes::Command(CcRpcTypes::Drop { .handle = handle });
		notify(args);
	}
}




void UsbCc::Handle::requestNextMove(uint32_t incoming) {
	auto args = CcRpcTypes::Command(CcRpcTypes::RequestNextMove { .handle = handle, .incoming = incoming });
	notify(args);
}


CcRpcTypes::PollNextMoveResult UsbCc::Handle::pollNextMove() {
	auto args = CcRpcTypes::Command(CcRpcTypes::PollNextMove { .handle = handle });
	return request<decltype(args), CcRpcTypes::PollNextMoveResult>(args);
}


std::optional<std::tuple<CcTypes::Move, CcTypes::MoveInfo>> UsbCc::Handle::blockNextMove() {
	auto args = CcRpcTypes::Command(CcRpcTypes::BlockNextMove { .handle = handle });
	return request<decltype(args), std::optional<std::tuple<CcTypes::Move, CcTypes::MoveInfo>>>(args);
}


void UsbCc::Handle::addNextPiece(CcTypes::Piece piece) {
	auto args = CcRpcTypes::Command(CcRpcTypes::AddNextPiece { .handle = handle, .piece = piece });
	notify(args);
}


void UsbCc::Handle::reset(std::array<std::array<bool, 10>, 40> field, bool b2b_active, uint32_t combo) {
	auto args = CcRpcTypes::Command(CcRpcTypes::Reset { .handle = handle, .field = field, .b2b_active = b2b_active, .combo = combo });
	notify(args);
}



// default things

CcTypes::Options UsbCc::defaultOptions() {
	auto args = CcRpcTypes::DefaultOptionsCommand();
	return request<decltype(args), CcTypes::Options>(args);
}


CcTypes::Evaluator UsbCc::defaultEvaluator() {
	auto args = CcRpcTypes::DefaultEvaluatorCommand();
	return request<decltype(args), CcTypes::Evaluator>(args);
}
