package main

import (
	"crypto/rand"
	"encoding/json"
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"math/big"
	"net/http"
	"os"
	"os/exec"
	"path"
	"path/filepath"
	"strconv"
	"strings"

	"github.com/rs/cors"
	"gopkg.in/yaml.v2"
)

var (
	mapsDir = flag.String("maps-dir", "maps", "Path to the directory where the maps are located")
	posDir  = flag.String("pos-dir", "pos", "Path to the directory where the positions are located (with a 'local' folder and a 'global' one within it)")
	test    = flag.Bool("test", false, "If set, returns random data at each call")
)

type Coords struct {
	X float64 `json:"x"`
	Y float64 `json:"y"`
}

type Pos struct {
	Origin         []float64 `yaml:"origin"`
	Position       []float64 `yaml:"position,omitempty"`
	Resolution     float64   `yaml:"resolution"`
	Negate         float64   `yaml:"negate"`
	OccupiedThresh float64   `yaml:"occupied_thresh"`
	FreeThresh     float64   `yaml:"free_thresh"`
}

type GlobalPosResp struct {
	Origin         Coords  `json:"origin"`
	Position       Coords  `json:"position"`
	Resolution     float64 `json:"resolution"`
	Negate         float64 `json:"negate"`
	OccupiedThresh float64 `json:"occupiedThresh"`
	FreeThresh     float64 `json:"freeThresh"`
}

func getRobotsNames() (robots []string, err error) {
	robots = make([]string, 0)
	fi, err := ioutil.ReadDir(*mapsDir)
	if err != nil {
		return
	}

	for _, file := range fi {
		filename := file.Name()
		extension := filepath.Ext(file.Name())
		if file.Name() != "global.png" && extension == ".png" {
			robots = append(robots, filename[0:len(filename)-len(extension)])
		}
	}

	return
}

func getPosForRobot(robotName string, global bool) (pos Pos, err error) {
	var n *big.Int
	var loc string

	if global {
		loc = "global"
	} else {
		loc = "local"
	}

	filename := robotName
	if *test {
		n, err = rand.Int(rand.Reader, big.NewInt(int64(3)))
		if err != nil {
			return
		}

		if n.Int64() != 0 {
			filename = robotName + "-" + strconv.Itoa(int(n.Int64()))
		}
	}

	filePath := path.Join(*posDir, loc, filename+".yaml")
	content, err := ioutil.ReadFile(filePath)
	if err != nil {
		return
	}

	if err = yaml.Unmarshal(content, &pos); err != nil {
		return
	}

	if len(pos.Origin) == 0 {
		pos.Origin = []float64{0, 0}
	}

	if len(pos.Position) == 0 {
		pos.Position = []float64{0, 0}
	}

	return
}

func main() {
	flag.Parse()

	mux := http.NewServeMux()
	mux.Handle("/maps/", http.StripPrefix("/maps/", http.FileServer(http.Dir(*mapsDir))))

	mux.HandleFunc("/robots", func(w http.ResponseWriter, req *http.Request) {
		var resp = struct {
			Robots []string `json:"robots"`
		}{
			Robots: []string{"global"},
		}

		robots, err := getRobotsNames()
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		resp.Robots = append(resp.Robots, robots...)

		content, err := json.Marshal(resp)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		w.Write(content)
	})

	mux.HandleFunc("/pos/local/", func(w http.ResponseWriter, req *http.Request) {
		robotName := strings.TrimPrefix(req.URL.Path, "/pos/local/")
		if robotName == "" {
			http.Error(w, "Robot name is empty", 400)
			return
		}

		pos, err := getPosForRobot(robotName, false)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		resp := struct {
			Origin         Coords  `json:"origin"`
			Position       Coords  `json:"position"`
			Resolution     float64 `json:"resolution"`
			Negate         float64 `json:"negate"`
			OccupiedThresh float64 `json:"occupiedThresh"`
			FreeThresh     float64 `json:"freeThresh"`
		}{
			Origin: Coords{
				X: pos.Origin[0],
				Y: pos.Origin[1],
			},
			Position: Coords{
				X: pos.Position[0],
				Y: pos.Position[1],
			},
			Resolution:     pos.Resolution,
			Negate:         pos.Negate,
			OccupiedThresh: pos.OccupiedThresh,
			FreeThresh:     pos.OccupiedThresh,
		}

		content, err := json.Marshal(resp)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		w.Write(content)
	})

	mux.HandleFunc("/pos/global", func(w http.ResponseWriter, req *http.Request) {
		var resp = make(map[string]GlobalPosResp)

		robotsNames, err := getRobotsNames()
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		for _, robotName := range robotsNames {
			pos, err := getPosForRobot(robotName, true)
			if err != nil {
				http.Error(w, err.Error(), 500)
				return
			}
			resp[robotName] = GlobalPosResp{
				Origin: Coords{
					X: pos.Origin[0],
					Y: pos.Origin[1],
				},
				Position: Coords{
					X: pos.Position[0],
					Y: pos.Position[1],
				},
				Resolution:     pos.Resolution,
				Negate:         pos.Negate,
				OccupiedThresh: pos.OccupiedThresh,
				FreeThresh:     pos.OccupiedThresh,
			}
		}

		content, err := json.Marshal(resp)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}

		w.Header().Set("Content-Type", "application/json")
		w.Write(content)
	})

	mux.HandleFunc("/map/", func(w http.ResponseWriter, req *http.Request) {
		robotName := strings.TrimPrefix(req.URL.Path, "/map/")
		if robotName == "" {
			http.Error(w, "Robot name is empty", 400)
			return
		}

		if req.Method != "POST" {
			http.Error(w, "Unsupported method, use POST instead", 400)
			return
		}

		req.ParseMultipartForm(32 << 20)
		file, handler, err := req.FormFile("map")
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}
		defer file.Close()

		ext := filepath.Ext(handler.Filename)
		if ext != ".png" && ext != ".pgm" {
			http.Error(w, "Unsupported file extenstion "+ext, 400)
			return
		}

		destPath := filepath.Join(*mapsDir, "raw", robotName+ext)
		if err = os.Remove(destPath); err != nil && !strings.Contains(err.Error(), "no such file or directory") {
			http.Error(w, err.Error(), 500)
			return
		}
		f, err := os.OpenFile(destPath, os.O_WRONLY|os.O_CREATE, 0666)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}
		defer f.Close()
		io.Copy(f, file)

		cmd := exec.Command(
			"convert",
			filepath.Join(*mapsDir, "raw", robotName+ext),
			filepath.Join(*mapsDir, robotName+".png"),
		)
		if output, err := cmd.Output(); err != nil {
			http.Error(w, fmt.Sprintf("%v: %s", err, string(output)), 500)
		}
	})

	mux.HandleFunc("/pos/", func(w http.ResponseWriter, req *http.Request) {
		robotName := strings.TrimPrefix(req.URL.Path, "/pos/")
		if robotName == "" {
			http.Error(w, "Robot name is empty", 400)
			return
		}

		if req.Method != "POST" {
			http.Error(w, "Unsupported method, use POST instead", 400)
			return
		}

		req.ParseMultipartForm(32 << 20)
		file, handler, err := req.FormFile("pos")
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}
		defer file.Close()

		ext := filepath.Ext(handler.Filename)
		if ext != ".yml" && ext != ".yaml" {
			http.Error(w, "Unsupported file extenstion "+ext, 400)
			return
		}

		destPath := filepath.Join(*posDir, "local", robotName+".yaml")
		if err = os.Remove(destPath); err != nil && !strings.Contains(err.Error(), "no such file or directory") {
			http.Error(w, err.Error(), 500)
			return
		}
		f, err := os.OpenFile(destPath, os.O_WRONLY|os.O_CREATE, 0666)
		if err != nil {
			http.Error(w, err.Error(), 500)
			return
		}
		defer f.Close()
		io.Copy(f, file)
	})

	handler := cors.Default().Handler(mux)
	if err := http.ListenAndServe("0.0.0.0:9090", handler); err != nil {
		panic(err)
	}
}
