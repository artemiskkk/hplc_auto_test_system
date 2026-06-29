from PIL import Image, ImageDraw, ImageFont
from math import atan2, cos, sin, pi
from pathlib import Path


OUT = Path(__file__).resolve().parent
W, H = 1800, 1060


def font(size, bold=False):
    name = "arialbd.ttf" if bold else "arial.ttf"
    return ImageFont.truetype(str(Path("C:/Windows/Fonts") / name), size)


F = {
    "title": font(38, True),
    "h1": font(28, True),
    "h2": font(23, True),
    "body": font(21),
    "small": font(18),
    "tiny": font(16),
    "bold": font(21, True),
}


class Diagram:
    def __init__(self, title):
        self.img = Image.new("RGB", (W, H), "white")
        self.d = ImageDraw.Draw(self.img)
        self.title = title
        self.d.text((W // 2, 28), title, anchor="mt", font=F["title"], fill="black")

    def text_size(self, text, ft):
        lines = text.split("\n")
        widths, heights = [], []
        for line in lines:
            box = self.d.textbbox((0, 0), line, font=ft)
            widths.append(box[2] - box[0])
            heights.append(box[3] - box[1])
        return max(widths) if widths else 0, sum(heights) + (len(lines) - 1) * 6

    def box(self, xy, text, ft=None, width=3, radius=7, fill="white", dashed=False):
        ft = ft or F["body"]
        x1, y1, x2, y2 = xy
        if dashed:
            self.dashed_rect(xy, width=width, radius=radius)
        else:
            self.d.rounded_rectangle(xy, radius=radius, fill=fill, outline="black", width=width)
        tw, th = self.text_size(text, ft)
        self.d.multiline_text(((x1 + x2) / 2, (y1 + y2) / 2 - th / 2), text, anchor="ma",
                              align="center", font=ft, fill="black", spacing=6)

    def group(self, xy, title, ft=None, width=3, radius=7):
        x1, y1, x2, y2 = xy
        self.d.rounded_rectangle(xy, radius=radius, fill="white", outline="black", width=width)
        self.d.text(((x1 + x2) / 2, y1 + 20), title, anchor="mt", font=ft or F["h2"], fill="black")

    def label(self, xy, text, ft=None, anchor="mm"):
        self.d.multiline_text(xy, text, anchor=anchor, align="center", font=ft or F["small"], fill="black", spacing=5)

    def dashed_rect(self, xy, width=2, radius=6, dash=14, gap=8):
        x1, y1, x2, y2 = xy
        self.d.rounded_rectangle(xy, radius=radius, outline="black", width=width)
        # Overdraw short white gaps to make a clean dashed look.
        for x in range(int(x1 + dash), int(x2), dash + gap):
            self.d.line((x, y1, min(x + gap, x2), y1), fill="white", width=width + 2)
            self.d.line((x, y2, min(x + gap, x2), y2), fill="white", width=width + 2)
        for y in range(int(y1 + dash), int(y2), dash + gap):
            self.d.line((x1, y, x1, min(y + gap, y2)), fill="white", width=width + 2)
            self.d.line((x2, y, x2, min(y + gap, y2)), fill="white", width=width + 2)

    def line(self, pts, width=3, dashed=False, arrow=True):
        if dashed:
            self.dashed_line(pts, width=width)
        else:
            self.d.line(pts, fill="black", width=width, joint="curve")
        if arrow:
            self.arrowhead(pts[-2], pts[-1], width=width)

    def dashed_line(self, pts, width=3, dash=16, gap=9):
        for a, b in zip(pts[:-1], pts[1:]):
            x1, y1 = a
            x2, y2 = b
            dx, dy = x2 - x1, y2 - y1
            dist = (dx * dx + dy * dy) ** 0.5
            if dist == 0:
                continue
            ux, uy = dx / dist, dy / dist
            t = 0
            while t < dist:
                t2 = min(t + dash, dist)
                self.d.line((x1 + ux * t, y1 + uy * t, x1 + ux * t2, y1 + uy * t2),
                            fill="black", width=width)
                t += dash + gap

    def arrowhead(self, p1, p2, width=3):
        x1, y1 = p1
        x2, y2 = p2
        ang = atan2(y2 - y1, x2 - x1)
        size = 17 + width
        left = (x2 - size * cos(ang - pi / 6), y2 - size * sin(ang - pi / 6))
        right = (x2 - size * cos(ang + pi / 6), y2 - size * sin(ang + pi / 6))
        self.d.polygon([p2, left, right], fill="black")

    def save(self, name):
        self.img.save(OUT / name, quality=95)


def architecture():
    g = Diagram("New Serial Host PC - Serial CCO Automation Architecture")

    # Main PC boundary.
    g.d.rounded_rectangle((45, 85, 1330, 700), radius=8, outline="black", width=4)
    g.label((690, 116), "New Serial Host PC", F["h1"])

    # Top workflow.
    top = [(90, 150, 350, 220, "UI: Test Tree"),
           (430, 150, 690, 220, "Test Scanner"),
           (770, 150, 1030, 220, "Test Runner"),
           (1100, 150, 1295, 220, "Report / Logs")]
    for b in top:
        g.box(b[:4], b[4], F["body"])
    g.line([(350, 185), (430, 185)])
    g.line([(690, 185), (770, 185)])
    g.line([(1030, 185), (1100, 185)])

    # Database block.
    g.group((75, 255, 375, 650), "DataBase CSV", F["h2"])
    g.dashed_rect((100, 305, 350, 630), width=2)
    csvs = ["TestCase.csv", "ConcentratorInfo.csv", "MeterInfo.csv", "SchemeInfo.csv", "DvcSerialInfo.csv", "ScriptPara"]
    y = 325
    for name in csvs:
        g.box((120, y, 330, y + 47), name, F["small"], width=2)
        y += 52

    # DLL block.
    g.group((450, 300, 780, 610), "SgHplcTestScript.dll", F["h2"])
    g.box((485, 395, 745, 465), "ReflectFactory", F["body"])
    g.box((485, 515, 745, 585), "TestCase Scripts", F["body"])
    g.line([(615, 465), (615, 515)], arrow=True)
    g.line([(630, 515), (630, 465)], arrow=True)

    # Host adapter block.
    g.group((855, 300, 1265, 610), "SerialCcoScriptHost", F["h2"])
    g.label((1060, 360), "AbstractScriptHost API", F["small"])
    g.box((885, 395, 1045, 475), "Serial\nManager", F["body"], width=2)
    g.box((1065, 395, 1235, 475), "Frame\nBuffer", F["body"], width=2)
    g.box((885, 500, 1045, 580), "Virtual\nFixture ACK", F["body"], width=2)
    g.box((1065, 500, 1235, 580), "Breaker\nAdapter", F["body"], width=2)

    # Internal data/control flows.
    g.line([(375, 455), (450, 455)])
    g.line([(780, 455), (855, 455)])
    g.line([(1040, 220), (1040, 265), (1030, 265), (1030, 300)])
    g.line([(260, 255), (260, 245), (560, 245), (560, 220)])
    g.line([(260, 255), (260, 240), (900, 240), (900, 300)])

    # Serial and HPLC physical chain. HPLC is deliberately between CCO and STA.
    g.box((835, 665, 1065, 735), "CCO COM Port\n(USB / Serial)", F["body"])
    g.box((835, 790, 1065, 860), "CCO\n(HPLC CCO Module)", F["body"])
    g.box((1085, 790, 1285, 860), "HPLC Network\n(PLC / Coupler)", F["body"])
    g.box((1305, 790, 1505, 860), "STA\n(Station Under Test)", F["body"])
    g.box((1530, 790, 1710, 860), "Real Meter", F["body"])
    g.line([(970, 610), (970, 665)])
    g.line([(950, 735), (950, 790)])
    g.line([(1065, 825), (1085, 825)])
    g.line([(1285, 825), (1305, 825)])
    g.line([(1505, 825), (1530, 825)])
    g.line([(1620, 860), (1620, 910), (805, 910), (805, 700), (835, 700)], dashed=True)
    g.label((1275, 895), "uplink response -> processMsg(CCO_GW)", F["tiny"])

    # Breaker side chain.
    g.box((1435, 300, 1665, 385), "Smart Breaker", F["body"])
    g.box((1415, 455, 1685, 520), "STA / Environment Power", F["body"])
    g.box((1415, 575, 1685, 640), "Not CCO Self Power", F["body"], dashed=True)
    g.line([(1235, 540), (1320, 540), (1320, 342), (1435, 342)], dashed=True)
    g.line([(1550, 385), (1550, 455)])
    g.line([(1550, 520), (1550, 575)], dashed=True)
    g.line([(1550, 520), (1550, 725), (1405, 725), (1405, 790)], dashed=True)

    # Scope and legend.
    g.box((1430, 105, 1680, 280), "Scope: 159 Cases\n----------------\nRun           140\nAdjust          15\nNot Covered      4", F["body"])
    g.box((60, 790, 500, 925), "", F["body"])
    g.line([(105, 830), (175, 830)], arrow=True)
    g.label((285, 830), "Normal script / data flow", F["small"])
    g.line([(105, 885), (175, 885)], dashed=True, arrow=True)
    g.label((335, 885), "Virtual ACK / optional control", F["small"])
    g.label((1170, 965), "Correct physical topology: CCO COM -> CCO -> HPLC Network -> STA -> Real Meter", F["bold"])
    g.save("serial-cco-architecture.png")


def execution_flow():
    g = Diagram("Serial CCO Test Execution Flow")
    xs = [70, 365, 660, 955, 1250, 1510]
    headers = ["Startup", "Scan", "Run Case", "Serial Session", "HPLC Network", "Result"]
    for x, h in zip(xs, headers):
        g.box((x, 110, x + 230, 165), h, F["h2"], fill="#f7f7f7")

    main = [
        (70, 235, 300, 305, "Load CSV\nDataBase"),
        (365, 235, 595, 305, "Scan\nTestCase.csv"),
        (660, 235, 890, 305, "ReflectFactory\ncreate script"),
        (660, 360, 890, 470, "setHost\naddAddrsInfo\nconfig"),
        (660, 535, 890, 605, "execute()"),
        (955, 535, 1185, 605, "sendMsg2Dvc\n(CCO_GW)"),
        (955, 675, 1185, 745, "Encode frame\nwrite COM"),
        (1250, 675, 1480, 745, "CCO -> HPLC\n376.2 / 645 / OOP"),
        (1510, 675, 1735, 745, "STA + Real Meter\nresponse"),
        (1510, 835, 1735, 905, "Report / Logs\nstatisticResult"),
    ]
    for b in main:
        g.box(b[:4], b[4], F["body"])

    # Main execution arrows.
    g.line([(300, 270), (365, 270)])
    g.line([(595, 270), (660, 270)])
    g.line([(775, 305), (775, 360)])
    g.line([(775, 470), (775, 535)])
    g.line([(890, 570), (955, 570)])
    g.line([(1070, 605), (1070, 675)])
    g.line([(1185, 710), (1250, 710)])
    g.line([(1480, 710), (1510, 710)])

    # Return path.
    g.line([(1625, 675), (1625, 635), (1070, 635), (1070, 605)], dashed=True)
    g.box((955, 380, 1185, 455), "Receive COM\nparse frame", F["body"])
    g.box((955, 235, 1185, 305), "processMsg\n(CCO_GW)", F["body"])
    g.line([(1070, 675), (1070, 455)], dashed=True)
    g.line([(1070, 380), (1070, 305)], dashed=True)
    g.line([(955, 270), (890, 270)], dashed=True)
    g.line([(775, 605), (775, 835), (1510, 835)], dashed=True)

    # Control branch.
    g.box((70, 535, 300, 605), "controlDvc()\nfrom scripts", F["body"])
    g.box((365, 535, 595, 605), "Compatibility\nAdapter", F["body"])
    g.box((365, 675, 595, 745), "Virtual ACK\nprocessCtrlDvcRes", F["body"])
    g.box((70, 675, 300, 745), "SetBaudRate\nlocal serial", F["body"])
    g.box((70, 835, 300, 905), "Power / Reset / Event\nvia Smart Breaker", F["body"])
    g.line([(660, 570), (595, 570)])
    g.line([(300, 570), (365, 570)])
    g.line([(480, 605), (480, 675)], dashed=True)
    g.line([(365, 710), (300, 710)], dashed=True)
    g.line([(365, 570), (300, 870)], dashed=True)
    g.line([(300, 870), (1250, 870), (1250, 745)], dashed=True)
    g.box((70, 945, 1735, 1015),
          "Execution rule: every controlDvc path must return a compatible ACK, otherwise BuildNetwork_GW state machine can stall.",
          F["bold"], fill="#f7f7f7")
    g.save("serial-cco-execution-flow.png")


def control_adapter():
    g = Diagram("BuildNetwork_GW controlDvc Adapter")

    g.box((70, 120, 430, 205), "Existing Scripts\nBuildNetwork_GW / other cases", F["h2"])
    g.box((70, 285, 450, 360), "controlDvc(dvcType, idList,\ncmd, params)", F["small"])
    g.box((70, 450, 430, 525), "Wait for processCtrlDvcRes\nsame dvcType / idList / cmd", F["body"])
    g.line([(250, 205), (250, 285)])
    g.line([(250, 360), (250, 450)])

    g.box((620, 120, 1000, 205), "SerialCcoScriptHost\nCompatibility Layer", F["h2"])
    g.box((620, 280, 1000, 360), "Decode command\npreserve script contract", F["body"])
    g.box((620, 450, 1000, 525), "Return virtual or real ACK\nprocessCtrlDvcRes", F["body"])
    g.line([(450, 322), (620, 322)])
    g.line([(810, 360), (810, 450)])
    g.line([(620, 487), (450, 487)], dashed=True)

    actions = [
        (1190, 120, 1735, 195, "SetBaudRate ->\nchange local QSerialPort baud"),
        (1190, 230, 1735, 305, "CCO PowerOn -> virtual ACK\nCCO already self-networked"),
        (1190, 340, 1735, 415, "CCO reset / crop ->\nprotocol reset or mark Adjust"),
        (1190, 450, 1735, 525, "STA power / reset / event ->\nCCO frame to Smart Breaker"),
        (1190, 560, 1735, 635, "Unsupported fixture-only behavior ->\nmark Not Covered"),
    ]
    for b in actions:
        g.box(b[:4], b[4], F["small"])
    for y in [157, 267, 377, 487, 597]:
        g.line([(1000, 322), (1090, 322), (1090, y), (1190, y)], dashed=True)

    g.box((250, 735, 440, 805), "Host PC", F["body"])
    g.box((520, 735, 730, 805), "CCO COM\nUSB / Serial", F["body"])
    g.box((810, 735, 1000, 805), "CCO", F["body"])
    g.box((1080, 735, 1280, 805), "HPLC Network", F["body"])
    g.box((1360, 735, 1540, 805), "STA", F["body"])
    g.box((1600, 735, 1760, 805), "Real Meter", F["body"])
    g.line([(440, 770), (520, 770)])
    g.line([(730, 770), (810, 770)])
    g.line([(1000, 770), (1080, 770)])
    g.line([(1280, 770), (1360, 770)])
    g.line([(1540, 770), (1600, 770)])

    g.box((1120, 875, 1370, 940), "Smart Breaker", F["body"])
    g.box((1440, 875, 1720, 940), "STA / Environment Power\nnot CCO self power", F["body"])
    g.line([(1450, 520), (1450, 660), (1235, 660), (1235, 875)], dashed=True)
    g.line([(1370, 908), (1440, 908)], dashed=True)
    g.line([(1580, 875), (1480, 805)], dashed=True)

    g.box((70, 890, 1060, 985),
          "Key point: scripts keep their original AbstractScriptHost contract.\nNew host replaces fixture I/O with serial CCO frames and virtual ACKs.",
          F["bold"], fill="#f7f7f7")
    g.save("buildnetwork-control-adapter.png")


if __name__ == "__main__":
    architecture()
    execution_flow()
    control_adapter()
    print("generated:")
    for name in [
        "serial-cco-architecture.png",
        "serial-cco-execution-flow.png",
        "buildnetwork-control-adapter.png",
    ]:
        print(OUT / name)
